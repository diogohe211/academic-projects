//Guilherme Filipe Reis Abadesso 2022216471
//Diogo da Silva Henriques 2019222685
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>

#include "common.h"
#include "utils.h"

#define SHM_KEY_POOL 1234
#define SEM_NAME "/transactiongen_sem"

TransactionPool *pool = NULL;
int shm_id = -1;
sem_t *sem = NULL;

int transaction_counter = 0;

void clean_up() {
    if (pool) shmdt(pool);
    if (sem) sem_close(sem);
}

void handle_sigint(int sig) {
    printf("\nTransaction Generator a terminar...\n");
    clean_up();
    exit(0);
}

int main(int argc, char *argv[]) {
    log_init();
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <reward (1-3)> <sleep_time_ms>\n", argv[0]);
        exit(1);
    }

    int reward = atoi(argv[1]);
    int sleep_time = atoi(argv[2]);

    if (reward < 1 || reward > 3 || sleep_time < 200 || sleep_time > 3000) {
        fprintf(stderr, "Parâmetros inválidos.\n");
        exit(1);
    }

    signal(SIGINT, handle_sigint);

    // Aceder à memória partilhada já criada pelo controller
    shm_id = shmget(SHM_KEY_POOL, sizeof(TransactionPool), 0666);
    if (shm_id == -1) {
        perror("shmget");
        exit(1);
    }

    pool = (TransactionPool *)shmat(shm_id, NULL, 0);
    if (pool == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    // Abrir (ou criar se necessário) o semáforo para sincronizar acesso à pool
    sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    srand(time(NULL) + getpid());

    while (1) {
        sem_wait(sem); // entrar em região crítica

        int inserted = 0;
        for (int i = 0; i < MAX_TRANSACTIONS; i++) {
            if (pool->transactions_pool[i].empty) {
                Transaction *t = &pool->transactions_pool[i];
                t->empty = 0;
                t->age = 0;
                t->reward = reward;
                t->value = rand() % 100 + 1;
                t->timestamp = time(NULL);

                char id[64];
                snprintf(id, sizeof(id), "%d-%d", getpid(), transaction_counter++);
                //log_message("TransactionGenerator criou transação [%s] com reward %d (pos %d)", id, reward, i);

                inserted = 1;
                break;
            }
        }

        sem_post(sem); // sair da região crítica

        if (!inserted) {
            //log_message("Transaction Pool cheia. Transação não inserida.");
        }

        usleep(sleep_time * 1000); // dormir em microsegundos
    }

    clean_up();
    log_close();
    return 0;
}
