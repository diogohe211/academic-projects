// statistics.c
//Guilherme Filipe Reis Abadesso 2022216471
//Diogo da Silva Henriques 2019222685
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include "common.h"
#include "utils.h"
#include "statistics.h"

#define MSG_QUEUE_KEY 5678
#define MAX_MINERS 100


volatile sig_atomic_t show_stats = 0;
volatile sig_atomic_t terminate = 0;


MinerStats miner_stats[MAX_MINERS];

void sigusr1_handler(int sig) {
    log_message("Statistics recebeu sinal SIGUSR1");
    show_stats = 1;
}

void termination_handler(int sig) {
    log_message("Statistics a terminar, sinal recebido: %d", sig);
    print_statistics();  // Imprimir antes de sair
    terminate = 1;
}

void print_statistics() {
    log_message("======= Estatísticas =======");
    for (int i = 0; i < MAX_MINERS; i++) {
        if (miner_stats[i].valid_blocks > 0 || miner_stats[i].invalid_blocks > 0) {
            log_message("Miner %d → Válidos: %d | Inválidos: %d | Créditos: %d",
                        i,
                        miner_stats[i].valid_blocks,
                        miner_stats[i].invalid_blocks,
                        miner_stats[i].total_credits);
        }
    }
    log_message("============================");
}

void process_stats_message(StatsMessage *msg) {
    if (msg->miner_id < 0 || msg->miner_id >= MAX_MINERS) return;

    if (msg->is_valid) {
        miner_stats[msg->miner_id].valid_blocks++;
        miner_stats[msg->miner_id].total_credits += msg->credits;
    } else {
        miner_stats[msg->miner_id].invalid_blocks++;
    }
}

void start_statistics() {
    log_init();

    signal(SIGUSR1, sigusr1_handler);
    signal(SIGINT, termination_handler);
    signal(SIGTERM, termination_handler);

    log_message("Processo Statistics iniciado. PID = %d", getpid());

    // Criar ou obter fila de mensagens
    msgid = msgget(MSG_QUEUE_KEY, IPC_CREAT | 0666);
    if (msgid == -1) {
        perror("msgget");
        exit(1);
    }

    StatsMessage msg;

    while (!terminate) {
        // Non-blocking receive
        if (msgrcv(msgid, &msg, sizeof(StatsMessage) - sizeof(long), 0, 0) != -1) {
            process_stats_message(&msg);
        }

        if (show_stats) {
            print_statistics();
            show_stats = 0;
        }

        usleep(500000);  // evitar busy waiting
    }

    // Remover fila de mensagens (se for o último a usar)
    msgctl(msgid, IPC_RMID, NULL);

    log_message("Processo Statistics terminou.");
    log_close();
}