// utils.h
//Guilherme Filipe Reis Abadesso 2022216471
//Diogo da Silva Henriques 2019222685
#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <semaphore.h>
#include "common.h"
extern FILE *global_log_file;
extern sem_t *global_log_sem;
void log_init();
void log_close();
void log_message(const char *format, ...);
int read_config(const char *filename, Config *config);

#endif
