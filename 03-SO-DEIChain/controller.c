// controller.c
//Guilherme Filipe Reis Abadesso 2022216471
//Diogo da Silva Henriques 2019222685
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <semaphore.h>
#include "common.h"
#include "utils.h"
#include "miner.h"
#include "validator.h"
#include "statistics.h"
#include <sys/msg.h>
#include <errno.h>
#include <pthread.h>

#define SHM_KEY_POOL 1234
#define SHM_KEY_LEDGER 5678
#define LOG_SEM_NAME "/log_sem"
#define FIFO_NAME "VALIDATOR_INPUT"

int fifo_dummy_fd = -1;
TransactionPool *pool = NULL;
BlockchainLedger *ledger = NULL;
int shm_id_pool = -1, shm_id_ledger = -1;
int msgid = -1;
pid_t miner_pid = -1, validator_pid = -1, stats_pid = -1;
volatile sig_atomic_t running = 1;
size_t transactions_per_block;

pid_t validator_pids[3] = {0}; // PIDs dos validators ativos
int active_validators = 1;     // Começa com 1 validator


void *manage_validators(void *arg) {
    Config *config = (Config *)arg;

    while (1) {
        int occupied = 0;
        for (int i = 0; i < config->tx_pool_size; i++) {
            if (!pool->transactions_pool[i].empty)
                occupied++;
        }

        double occupancy = (double)occupied / config->tx_pool_size;

        // Escalar para 2 Validators
        if (occupancy >= 0.6 && active_validators < 2) {
            pid_t pid = fork();
            if (pid == 0) {
                start_validator();
                exit(0);
            } else if (pid > 0) {
                validator_pids[active_validators] = pid;
                active_validators++;
                log_message("Controller: Lançado segundo Validator (ocupação %.2f)", occupancy);
            }
        }

        // Escalar para 3 Validators
        else if (occupancy >= 0.8 && active_validators < 3) {
            pid_t pid = fork();
            if (pid == 0) {
                start_validator();
                exit(0);
            } else if (pid > 0) {
                validator_pids[active_validators] = pid;
                active_validators++;
                log_message("Controller: Lançado terceiro Validator (ocupação %.2f)", occupancy);
            }
        }

        // Reduzir para menos Validators se < 40%
        else if (occupancy < 0.4 && active_validators > 1) {
            pid_t to_kill = validator_pids[active_validators - 1];
            if (to_kill > 0) {
                kill(to_kill, SIGTERM);
                waitpid(to_kill, NULL, 0);
                log_message("Controller: Terminou Validator extra (ocupação %.2f)", occupancy);
                validator_pids[active_validators - 1] = 0;
                active_validators--;
            }
        }

        sleep(1);
    }

    return NULL;
}

void clean_up() {
    for (int i = 0; i < active_validators; i++) {
    if (validator_pids[i] > 0) {
        kill(validator_pids[i], SIGTERM);
        waitpid(validator_pids[i], NULL, 0);
    }
}
    if (pool) shmdt(pool);
    if (shm_id_pool != -1) shmctl(shm_id_pool, IPC_RMID, NULL);
    if (ledger) shmdt(ledger);
    if (shm_id_ledger != -1) shmctl(shm_id_ledger, IPC_RMID, NULL);
    sem_unlink(LOG_SEM_NAME);
    sem_unlink("/transactiongen_sem");
    msgctl(msgid, IPC_RMID, NULL);
    log_message("Message queue removida pelo controller.");
    if (fifo_dummy_fd != -1) {
    close(fifo_dummy_fd);
    }
    unlink(FIFO_NAME);
    log_message("FIFO removida.");
    log_message("Recursos libertados e simulação terminada.");
    
    log_close();
}

void handle_sigint(int sig) {
    log_message("SIGINT recebido no Controller. A terminar...");

    if (miner_pid > 0) kill(miner_pid, SIGINT);
    if (validator_pid > 0) kill(validator_pid, SIGINT);
    if (stats_pid > 0) kill(stats_pid, SIGINT);

    clean_up();
    exit(0);
}

int main() {

    log_init();
    log_message("Controller iniciado.");
    signal(SIGINT, handle_sigint);

    Config config;
    if (read_config("config.cfg", &config) != 0) {
        log_message("Erro ao ler config.cfg");
        exit(1);
    }

    log_message("Configurações: Miners=%d, PoolSize=%d, TxPerBlock=%d, MaxBlocks=%d",
                config.num_miners, config.tx_pool_size,
                config.transactions_per_block, config.blockchain_blocks);
    transactions_per_block = config.transactions_per_block;

    sem_unlink("/transactiongen_sem");
    sem_t *sem = sem_open("/transactiongen_sem", O_CREAT | O_EXCL, 0644, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open /transactiongen_sem no controller");
        exit(1);
    }
    sem_close(sem);
    log_message("Semáforo /transactiongen_sem criado pelo controller.");

    shm_id_pool = shmget(SHM_KEY_POOL, sizeof(TransactionPool), IPC_CREAT | 0666);
    if (shm_id_pool == -1) {
        perror("shmget pool");
        exit(1);
    }
    pool = (TransactionPool *)shmat(shm_id_pool, NULL, 0);
    if (pool == (void *) -1) {
        perror("shmat pool");
        exit(1);
    }
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        pool->transactions_pool[i].empty = 1;
    }
    pool->current_block_id = 0;
    log_message("Transaction Pool inicializada.");

    shm_id_ledger = shmget(SHM_KEY_LEDGER, sizeof(BlockchainLedger), IPC_CREAT | 0666);
    if (shm_id_ledger == -1) {
        perror("shmget ledger");
        exit(1);
    }
    ledger = (BlockchainLedger *)shmat(shm_id_ledger, NULL, 0);
    memset(ledger, 0, sizeof(BlockchainLedger));
    log_message("Memória partilhada para Blockchain Ledger criada.");

    msgid = msgget(MSG_QUEUE_KEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget controller");
        clean_up();
        exit(1);
    }
    log_message("Message queue criada com sucesso pelo controller.");

    unlink(FIFO_NAME);  // Apaga qualquer ficheiro antigo com esse nome (FIFO ou não)
    if (mkfifo(FIFO_NAME, 0666) == -1) {
        perror("mkfifo");
        clean_up();
        exit(1);
    }

    
    log_message("FIFO criada: %s", FIFO_NAME);
    
    validator_pid = fork();
    if (validator_pid < 0) {
        log_message("Erro: fork() falhou ao criar o processo Validator.");
        kill(miner_pid, SIGINT); // Termina o processo anterior
        clean_up();
        exit(1);
    } else if (validator_pid == 0) {
        start_validator();
        exit(0);
    }
    //  thread que gere os restantes validators
    pthread_t validator_manager;
    pthread_create(&validator_manager, NULL, manage_validators, (void *)&config);

    miner_pid = fork();
    if (miner_pid < 0) {
        log_message("Erro: fork() falhou ao criar o processo Miner.");
        clean_up();
        exit(1);
    } else if (miner_pid == 0) {
        start_miner();
        exit(0);
    }

    

    stats_pid = fork();
    if (stats_pid < 0) {
        log_message("Erro: fork() falhou ao criar o processo Statistics.");
        kill(miner_pid, SIGINT);
        kill(validator_pid, SIGINT);
        clean_up();
        exit(1);
    } else if (stats_pid == 0) {
        start_statistics();
        exit(0);
}

    log_message("Processos Miner, Validator e Statistics criados.");

    wait(NULL); wait(NULL); wait(NULL);

    clean_up();
    return 0;
}