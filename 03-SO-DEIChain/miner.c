// miner.c 
//Guilherme Filipe Reis Abadesso 2022216471
//Diogo da Silva Henriques 2019222685
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "common.h"
#include "utils.h"
#include "miner.h"
#include "pow.h"


#define SHM_KEY_POOL 1234
#define SHM_KEY_LEDGER 5678
#define MSG_QUEUE_KEY 5678
#define FIFO_NAME "VALIDATOR_INPUT"

extern volatile sig_atomic_t running;

void clean_up_miner() {
    log_message("Miner: recursos limpos");
    if (pool) shmdt(pool);
    if (ledger) shmdt(ledger);
    log_close();
}

void handle_sigint_miner(int sig) {
    log_message("Miner recebeu SIGINT, a terminar processo...");
    running = 0;
    sleep(1);
    clean_up_miner();
    exit(0);
}

void *miner_thread_func(void *arg) {
    int id = *((int *)arg);
    log_message("Thread Miner %d iniciada", id);

    Config config;
    if (read_config("config.cfg", &config) != 0) {
        log_message("Miner: Erro ao ler config.cfg");
        exit(1);
    }

    while (running) {
        Block bloco;
        snprintf(bloco.txb_id, TXB_ID_LEN, "%d", pool->current_block_id++);
        bloco.miner_id = id;
        bloco.num_transactions = 0;
        bloco.valid = 1;
        bloco.nonce = 0;

        // Obter hash do bloco anterior
        char previous_hash[HASH_SIZE] = INITIAL_HASH;
        for (int i = 0; i < config.blockchain_blocks; i++) {
            if (ledger->blocks[i].txb_id != 0) {
                compute_sha256(&ledger->blocks[i], previous_hash);
            }
        }
        strncpy(bloco.previous_block_hash, previous_hash, HASH_SIZE);

        // Preencher transações
        for (int i = 0; i < MAX_TRANSACTIONS && bloco.num_transactions < config.transactions_per_block; i++) {
            if (!pool->transactions_pool[i].empty) {
                bloco.transactions[bloco.num_transactions++] = pool->transactions_pool[i];
                pool->transactions_pool[i].empty = 1;
            }
        }

        if (bloco.num_transactions == 0) {
            usleep(2000000);
            continue;
        }

        // Realizar PoW
        PoWResult r;
        do {
            bloco.timestamp = time(NULL);
            r = proof_of_work(&bloco);
            if (r.error) {
                log_message("Miner %d: falhou PoW (timestamp = %ld), a tentar novamente", id, bloco.timestamp);
            }
        } while (r.error == 1);

        log_message("Miner %d: PoW concluído para bloco %d (hash: %s, nonce: %u)", id, bloco.txb_id, r.hash, bloco.nonce);

        if (!verify_nonce(&bloco)) {
            log_message("Miner %d: nonce inválido para bloco %d. Bloco descartado.", id, bloco.txb_id);
            continue;
        }

        // Abrir o FIFO para escrita (modo bloqueante)
        int fifo_fd = open(FIFO_NAME, O_WRONLY| O_NONBLOCK);
        if (fifo_fd == -1) {
            perror("Miner: erro ao abrir FIFO para escrita");
            pthread_exit(NULL); // ou exit(1); se estiveres fora da thread
        }

        // Enviar o bloco para o validator
        ssize_t bytes_written = write(fifo_fd, &bloco, sizeof(Block));
        if (bytes_written != sizeof(Block)) {
            perror("Miner: erro ao escrever bloco no FIFO");
        } else {
            log_message("Miner %d: bloco %s enviado para validator", id, bloco.txb_id);
        }

    close(fifo_fd); // Fecha o FIFO após envio

        // Enviar mensagem para estatísticas (assume válido até validação)
        StatsMessage msg;
        msg.mtype = 1;
        msg.miner_id = id;
        msg.is_valid = 1;  // assumido
        msg.credits = bloco.num_transactions;

        int msgid = msgget(MSG_QUEUE_KEY, IPC_CREAT | 0666);
        if (msgid != -1) {
            msgsnd(msgid, &msg, sizeof(StatsMessage) - sizeof(long), 0);
        }

        usleep(2000000);
    }

    return NULL;
}

void start_miner() {
    signal(SIGINT, handle_sigint_miner);
    log_init();
    log_message("Processo Miner iniciado");
    Config config;
    if (read_config("config.cfg", &config) != 0) {
        log_message("Miner: Erro ao ler config.cfg");
        exit(1);
    }
    int NUM_MINERS = config.num_miners;
    shm_id_pool = shmget(SHM_KEY_POOL, sizeof(TransactionPool), 0666);
    if (shm_id_pool == -1) {
        perror("shmget miner");
        log_close();
        exit(1);
    }
    pool = (TransactionPool *)shmat(shm_id_pool, NULL, 0);
    if (pool == (void *) -1) {
        perror("shmat miner");
        log_close();
        exit(1);
    }

    shm_id_ledger = shmget(SHM_KEY_LEDGER, sizeof(BlockchainLedger), 0666);
    if (shm_id_ledger == -1) {
        perror("shmget ledger miner");
        log_close();
        exit(1);
    }
    ledger = (BlockchainLedger *)shmat(shm_id_ledger, NULL, 0);
    if (ledger == (void *) -1) {
        perror("shmat ledger miner");
        log_close();
        exit(1);
    }

    
    pthread_t threads[NUM_MINERS];
    int ids[NUM_MINERS];

    for (int i = 0; i < NUM_MINERS; i++) {
        ids[i] = i;
        if (pthread_create(&threads[i], NULL, miner_thread_func, &ids[i]) != 0) {
            perror("Erro ao criar thread miner");
            log_close();
            exit(1);
        }
    }

    for (int i = 0; i < NUM_MINERS; i++) {
        pthread_join(threads[i], NULL);
    }

    clean_up_miner();
}
