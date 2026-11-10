//Guilherme Filipe Reis Abadesso 2022216471
//Diogo da Silva Henriques 2019222685
//Estruturas e configurações partilhadas
#ifndef COMMON_H
#define COMMON_H

#include <time.h>
#include <sys/types.h>
#define MSG_QUEUE_KEY 5678
#define FIFO_NAME "VALIDATOR_INPUT"
#define MAX_TRANSACTIONS 100
#define MAX_TRANSACTIONS_PER_BLOCK 10  // será ajustado pelo config
#define TX_ID_LEN 64
#define TXB_ID_LEN 64
#define HASH_SIZE 65  // SHA256_DIGEST_LENGTH * 2 + 1
extern int msgid;
extern size_t transactions_per_block;
// ALTERAR NR DE TRANSAÇÕES PARA O VALOR DO FICH DE CONFIG 
//estrutura de uma transação
typedef struct {
    int empty; //indica se o slot da pool está ocupado.
    int age; //idade da transação
    int reward;// recompensa da transação 1-3
    char tx_id[TX_ID_LEN];  // Unique transaction ID (e.g., PID + #)
    int value; //valor da transação
    time_t timestamp; //quando foi enviada
} Transaction;
typedef struct {
    char txb_id[TXB_ID_LEN]; 
    int miner_id;
    int num_transactions;
    int valid;
    Transaction transactions[MAX_TRANSACTIONS_PER_BLOCK];
    char previous_block_hash[HASH_SIZE];
    char hash[HASH_SIZE];
    unsigned int nonce;
    time_t timestamp;
} Block;
static inline size_t get_transaction_block_size() {
  if (transactions_per_block == 0) {
    perror("Must set the 'transactions_per_block' variable before using!\n");
    exit(-1);
  }
  return sizeof(Block) +
         transactions_per_block * sizeof(Transaction);
}

//memoria partilhada para guardar transações geradas por transactiongenerator
typedef struct {
    char current_block_id;
    Transaction transactions_pool[MAX_TRANSACTIONS];
} TransactionPool;

// Mensagem para comunicação estatística via message queue
typedef struct {
    long mtype;
    int miner_id;
    int is_valid;
    int credits;
} StatsMessage;

typedef struct {
    int valid_blocks;
    int invalid_blocks;
    int total_credits;
} MinerStats;



//Estrutura base para guardar os blocos válidos que compõem a blockchain.
typedef struct {
    Block blocks[MAX_TRANSACTIONS];
} BlockchainLedger;

// Configuração Guarda os valores lidos do ficheiro config.cfg
typedef struct {
    int num_miners;
    int tx_pool_size;
    int transactions_per_block;
    int blockchain_blocks;
} Config;



// Declarações globais para partilha entre módulos (definidas no controller.c)
extern TransactionPool *pool;
extern BlockchainLedger *ledger;
extern int shm_id_pool;
extern int shm_id_ledger;

#endif
