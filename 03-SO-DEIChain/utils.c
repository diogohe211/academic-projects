// utils.c
//Guilherme Filipe Reis Abadesso 2022216471
//Diogo da Silva Henriques 2019222685
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>

#define LOG_FILE "DEIChain_log.txt"
#define LOG_SEM_NAME "/log_sem"

#include <stdio.h>
#include <semaphore.h>

FILE *global_log_file = NULL;
sem_t *global_log_sem = NULL;

void log_init() {
    global_log_sem = sem_open(LOG_SEM_NAME, O_CREAT, 0644, 1);
    if (global_log_sem == SEM_FAILED) {
        perror("sem_open LOG");
        exit(1);
    }

    global_log_file = fopen("DEIChain_log.txt", "a");
    if (!global_log_file) {
        perror("fopen LOG");
        sem_close(global_log_sem);
        exit(1);
    }
}

void log_close() {
    if (global_log_file) {
        fclose(global_log_file);
        global_log_file = NULL;
    }

    if (global_log_sem) {
        sem_close(global_log_sem);
        global_log_sem = NULL;
    }

    sem_unlink(LOG_SEM_NAME);
}

/*Escreve mensagens com timestamp no ecrã (stdout) e no ficheiro de log DEIChain_log.txt 
Usa va_list para permitir mensagens com múltiplos argumentos (como printf).
*/
void log_message(const char *format, ...) {
    if (!global_log_file || !global_log_sem) return;

    sem_wait(global_log_sem);

    time_t now = time(NULL);
    char *time_str = ctime(&now);
    time_str[strlen(time_str) - 1] = '\0'; // remove newline

    printf("[%s] ", time_str);
    fprintf(global_log_file, "[%s] ", time_str);

    va_list args;
    va_start(args, format);

    va_list args_copy;
    va_copy(args_copy, args);

    vprintf(format, args);
    vfprintf(global_log_file, format, args_copy);

    va_end(args);
    va_end(args_copy);

    printf("\n");
    fprintf(global_log_file, "\n");
    fflush(global_log_file); // garante escrita imediata

    sem_post(global_log_sem);
}


/*
Lê os 4 parâmetros do ficheiro config.cfg e guarda no Config.
Retorna 0 se tudo correr bem, -1 se falhar.
TODO: VERIFICAR SE O FICHEIRO EXISTE E TEM OS VALORES CERTO.
 */
int read_config(const char *filename, Config *config) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        log_message("Erro: Não foi possível abrir o ficheiro de configuração '%s'.", filename);
        return -1;
    }

    int ret = fscanf(file, "%d\n%d\n%d\n%d",
                     &config->num_miners,
                     &config->tx_pool_size,
                     &config->transactions_per_block,
                     &config->blockchain_blocks);
    fclose(file);
    if (ret != 4) {
        log_message("Erro: O ficheiro de configuração não contém 4 valores válidos.");
        return -1;
    }

    if (config->num_miners <= 0 || config->tx_pool_size <= 0 ||
        config->transactions_per_block <= 0 || config->blockchain_blocks <= 0) {
        log_message("Erro: Parâmetros inválidos no ficheiro de configuração. Todos devem ser > 0.");
        return -1;
    }

    if (config->transactions_per_block > config->tx_pool_size) {
        log_message("Erro: TRANSACTIONS_PER_BLOCK não pode ser maior que TX_POOL_SIZE.");
        return -1;
    }



    return 0;
}
