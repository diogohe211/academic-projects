// validator.c 
//Guilherme Filipe Reis Abadesso 2022216471
//Diogo da Silva Henriques 2019222685
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include "common.h"
#include "utils.h"
#include "validator.h"
#include "pow.h"
#include <semaphore.h>

#define SHM_KEY_LEDGER 5678

extern volatile sig_atomic_t running;

void aplicar_aging() {
    sem_t *sem = sem_open("/transactiongen_sem", 0);
    if (sem == SEM_FAILED) {
        perror("sem_open validator (aging)");
        return;
    }
    sem_wait(sem);
    for (int i = 0; i < MAX_TRANSACTIONS; i++) {
        if (!pool->transactions_pool[i].empty) {
            pool->transactions_pool[i].age++;
            if (pool->transactions_pool[i].age % 50 == 0) {
                pool->transactions_pool[i].reward++;
                /*log_message("Validator: transação ID %s com age %d → reward incrementado para %d",
                            pool->transactions_pool[i].tx_id,
                            pool->transactions_pool[i].age,
                            pool->transactions_pool[i].reward);*/
            }
        }
    }
    sem_post(sem);
    sem_close(sem);
}

void handle_sigint_validator(int sig) {
    log_message("Validator recebeu SIGINT, a terminar...");
    running = 0;
    sleep(1);
    log_close();
    exit(0);
}

void start_validator() {
    signal(SIGINT, handle_sigint_validator);
    log_init();
    log_message("Processo Validator iniciado");
    sem_t *sem = sem_open("/transactiongen_sem", 0);
    if (sem == SEM_FAILED) {
        perror("sem_open validator");
        log_close();
        exit(1);
    }

    shm_id_ledger = shmget(SHM_KEY_LEDGER, sizeof(BlockchainLedger), 0666);
    if (shm_id_ledger == -1) {
        perror("shmget validator");
        log_close();
        exit(1);
    }

    ledger = (BlockchainLedger *)shmat(shm_id_ledger, NULL, 0);
    if (ledger == (void *) -1) {
        perror("shmat validator");
        log_close();
        exit(1);
    }

    int fifo_fd = open(FIFO_NAME, O_RDONLY);
    if (fifo_fd == -1) {
        perror("Validator: erro ao abrir FIFO");
        log_close();
        exit(1);
    }
    msgid = msgget(MSG_QUEUE_KEY, 0666);
    if (msgid == -1) {
        perror("msgget validator");
        exit(1);
    }

    Config config;
    if (read_config("config.cfg", &config) != 0) {
        log_message("Validator: Erro ao ler config.cfg");
        close(fifo_fd);
        log_close();
        exit(1);
    }


    while (running) {
        aplicar_aging();  // Atualiza aging e rewards
        Block bloco;
        ssize_t n = read(fifo_fd, &bloco, sizeof(Block));

        sem_wait(sem);
        for (int i = 0; i < MAX_TRANSACTIONS; i++) {
            if (!pool->transactions_pool[i].empty) {
                pool->transactions_pool[i].age++;
                if (pool->transactions_pool[i].age % 50 == 0) {
                    pool->transactions_pool[i].reward++;
                }
            }
        }
        sem_post(sem);

        if (n == sizeof(Block)) {
            log_message("Validator: bloco %s recebido do miner", bloco.txb_id);
            bloco.valid = 1;

            // Verificação simplificada usando tx_id
            for (int i = 0; i < bloco.num_transactions && bloco.valid; i++) {
                if (strlen(bloco.transactions[i].tx_id) == 0) {
                    bloco.valid = 0;
                    log_message("Validator: transação %d com ID vazio no bloco %s", i, bloco.txb_id);
                }
            }

            if (!verify_nonce(&bloco)) {
                log_message("Validator: nonce inválido para bloco %s", bloco.txb_id);
                bloco.valid = 0;
            }
            // Confirmar que o previous_block_hash corresponde ao último bloco válido
            char ultimo_hash[HASH_SIZE] = INITIAL_HASH;
            for (int i = 0; i < config.blockchain_blocks; i++) {
                if (ledger->blocks[i].txb_id[0] != '\0') {
                    compute_sha256(&ledger->blocks[i], ultimo_hash);
                } else {
                    break;
                }
            }
            if (strncmp(bloco.previous_block_hash, ultimo_hash, HASH_SIZE) != 0) {
                log_message("Validator: previous_block_hash inválido para bloco %s", bloco.txb_id);
                bloco.valid = 0;
                StatsMessage stats;
                stats.mtype = 1;
                stats.miner_id = bloco.miner_id;
                stats.is_valid = bloco.valid;
                stats.credits = 0;

                if (bloco.valid) {
                    for (int i = 0; i < bloco.num_transactions; i++) {
                        stats.credits += bloco.transactions[i].reward;
                    }
                }
                if (msgsnd(msgid, &stats, sizeof(StatsMessage) - sizeof(long), 0) == -1) {
                    perror("Validator: erro ao enviar mensagem de estatísticas");
                }
            }

            // Verificar se há espaço na ledger
            if (!bloco.valid) {
                // Enviar mensagem de estatísticas mesmo para blocos inválidos
                StatsMessage stats;
                stats.mtype = 1;
                stats.miner_id = bloco.miner_id;
                stats.is_valid = 0;
                stats.credits = 0;
                if (msgsnd(msgid, &stats, sizeof(StatsMessage) - sizeof(long), 0) == -1) {
                    perror("Validator: erro ao enviar mensagem de estatísticas");
                }

                log_message("Validator: bloco %s rejeitado. Não será inserido na ledger.", bloco.txb_id);
                continue;
            }

            // Verificar se há espaço na ledger
            int pos_livre = -1;
            for (int i = 0; i < config.blockchain_blocks; i++) {
                if (ledger->blocks[i].txb_id[0] == '\0') {
                    pos_livre = i;
                    break;
                }
            }

            if (pos_livre == -1) {
                log_message("Validator: Ledger cheia. Bloco %s não inserido.", bloco.txb_id);
                pid_t controller_pid = getppid();
                kill(controller_pid, SIGINT);
                running = 0;
                continue;
            }

            // Inserção final do bloco válido
            ledger->blocks[pos_livre] = bloco;
            log_message("Validator: bloco %s inserido na posição %d da ledger", bloco.txb_id, pos_livre);

            // Enviar estatísticas de bloco válido
            StatsMessage stats;
            stats.mtype = 1;
            stats.miner_id = bloco.miner_id;
            stats.is_valid = 1;
            stats.credits = 0;
            for (int i = 0; i < bloco.num_transactions; i++) {
                stats.credits += bloco.transactions[i].reward;
            }
            if (msgsnd(msgid, &stats, sizeof(StatsMessage) - sizeof(long), 0) == -1) {
                perror("Validator: erro ao enviar mensagem de estatísticas");
            }

            sem_wait(sem); // Região crítica da pool
            // Remover transações do pool
            for (int i = 0; i < bloco.num_transactions; i++) {
                for (int j = 0; j < MAX_TRANSACTIONS; j++) {
                    if (!pool->transactions_pool[j].empty &&
                        strcmp(pool->transactions_pool[j].tx_id, bloco.transactions[i].tx_id) == 0) {
                        pool->transactions_pool[j].empty = 1;
                        break;
                    }
                }
            }
            sem_post(sem);
        }
    }

    close(fifo_fd);
    shmdt(ledger);
    log_close();
}
