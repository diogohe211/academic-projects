#ifndef HASH_H_H
#define HASH_H_H

#define FREEIF(p) if(p){free(p);p=0;}

//The element structure of the hash table
typedef struct _StHash_Item
{	
	char *pKey;							//The key value of the hash table
	void *pMatch_msg;						//The value of the hash table
	struct _StHash_Item *pNext;			//Pointer to the next item in the hash table
}StHash_Item;

//The structure of the hash table
typedef struct
{
	int nBucket_size;							//The size of the hash table (similar to the head pointer size of the linked list)
	int nHashtable_len;						//Number of elements
	StHash_Item **ppItem;					//Elements of the hash table (number of head pointers)
	int    (*hash_func)(char *);	 //Define a function pointer
	pthread_mutex_t hash_mutex;
}StHash_Table;

int HashInit(StHash_Table **t, int size, int (*hash_func)(char *));
StHash_Item *HashGetItem(StHash_Table *pHash_table, char *pKey);		//Get an item of the hash table
void FreeFunc(void * data);
int  HashFunc(char *key);
void *HashData(StHash_Table *t, char *key);
void *HashDel(StHash_Table **t, char *key);
int HashFree(StHash_Table **t, void (*free_func)(void *));
int HashInsert(StHash_Table **t, char *key, void *data);
int IsHashEmpty(StHash_Table *pHash_table);
int HashtableLen(StHash_Table *pHashtable);

#endif
