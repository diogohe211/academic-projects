# Compilador e flags
CC = gcc
CFLAGS = -Wall -pthread -lrt

# Alvo principal
all: controller transactionGenerator

# Compilar controller com os outros .c para satisfazer as dependências
controller: controller.c utils.o miner.o validator.o statistics.o pow.o
	$(CC) $(CFLAGS) controller.c utils.o miner.o validator.o statistics.o pow.o -o controller -lcrypto

# Compilar transactionGenerator
transactionGenerator: transactionGenerator.c utils.o
	$(CC) $(CFLAGS) transactionGenerator.c utils.o -o transactionGenerator

# Objetos necessários
utils.o: utils.c utils.h common.h
	$(CC) $(CFLAGS) -c utils.c -o utils.o

miner.o: miner.c miner.h
	$(CC) $(CFLAGS) -c miner.c -o miner.o

validator.o: validator.c validator.h
	$(CC) $(CFLAGS) -c validator.c -o validator.o

statistics.o: statistics.c statistics.h
	$(CC) $(CFLAGS) -c statistics.c -o statistics.o

pow.o: pow.c pow.h
	$(CC) $(CFLAGS) -c pow.c -o pow.o
clean:
	rm -f *.o controller transactionGenerator
