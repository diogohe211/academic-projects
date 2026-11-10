
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

#include "hash.h"

/******This file is mainly used to manipulate the various operations of the hash table**************/

//Function purpose: release memory function
//Input parameter: variable pointer to be released
//Output parameters: None
//Return value: None
void FreeFunc(void *pData)
{
	FREEIF(pData);
}


//Function purpose: Get the subscript of the hash table entry
//Input parameter: key value
//Output parameters: None
//Return value: the index of the hash table entry
int  HashFunc(char *pKey)
{
	unsigned int nIdx = 0; 
	while (*pKey) 
	{
		nIdx ^= *pKey++;
		nIdx  <<= 1;
	}

	return nIdx;
}



//function;: Initialize the hash table
//args: Hash table pointer pointer, hash table size, hash table default function pointer
//Output: None
//return	: Initialization success, return 0, initialization failure, return -1
int HashInit(StHash_Table **ppHashtable, int nSize, int (*hash_func)(char *))
{
	if (nSize <= 0)
	{
		printf("HashInit: size<=0 func param error\n");
		return -1;   
	}	
	
	(*ppHashtable) = (StHash_Table *)malloc(sizeof(StHash_Table));
	if (NULL == (*ppHashtable))
	{
		printf("HashInit: Call malloc error\n");
		return -1;
	}
	
	memset((*ppHashtable), 0, sizeof(StHash_Table));
	if (pthread_mutex_init(&(*ppHashtable)->hash_mutex, NULL) != 0)
	{
		printf("HashInit: Call pthread_mutex_init error\n");
		return -1;
	}
	
	(*ppHashtable)->nBucket_size = nSize;
	
	(*ppHashtable)->ppItem = (StHash_Item **)malloc(sizeof(StHash_Item *) * nSize);
	if (NULL == (*ppHashtable)->ppItem) 
	{
		printf("HashInit: Call malloc error\n");
		(*ppHashtable)->nBucket_size = 0;
		FREEIF(*ppHashtable);
		return -1;
	}

	memset((*ppHashtable)->ppItem, 0, sizeof(StHash_Item *) * nSize);

	int i = 0;
	for (i=0; i<nSize; i++)
	{
		(*ppHashtable)->ppItem[i] = NULL;
	}
	
	if (hash_func)
	{
		(*ppHashtable)->hash_func = hash_func;
	}
	else
	{
		(*ppHashtable)->hash_func = HashFunc;  
	}

	
	(*ppHashtable)->nHashtable_len = 0;
	return 0;
}

//Function purpose: Get the corresponding hash table item according to the key of the hash table
//Input parameters: pointer to hash table, key pointer to hash table
//Output parameters: None
//Return value: Get successful, return the obtained numeric address, get failed, return NULL
StHash_Item *HashGetItem(StHash_Table *pHash_table, char *pKey)	
{
	unsigned int nIdx = 0;
	StHash_Item *pItem = NULL;
	if (NULL ==  pHash_table|| pHash_table->nBucket_size <= 0 || NULL == pHash_table->hash_func || NULL == pKey)
	{
		printf("HashGetItem: func param error\n");
		return NULL; 
	}

	pthread_mutex_lock(&pHash_table->hash_mutex);
	nIdx = pHash_table->hash_func(pKey);
	nIdx %= pHash_table->nBucket_size;
	for (pItem=pHash_table->ppItem[nIdx]; pItem!=NULL; pItem=pItem->pNext)
	{
		if (0 == strcmp(pKey, pItem->pKey))
		{
			pthread_mutex_unlock(&pHash_table->hash_mutex);
			return pItem;
		}
	}

	pthread_mutex_unlock(&pHash_table->hash_mutex);
	return NULL;
}

//Function purpose: release the dynamically allocated memory of the entire hash table
//Input parameters: pointer to hash table pointer, hash table default function pointer
//Output parameters: None
//Return value: if the memory is released successfully, it returns 0, if the release fails, it returns -1
int HashFree(StHash_Table **ppHashtable, void (*free_func)(void *))
{
	int i = 0;
	StHash_Item *pItem = NULL;
	StHash_Item *pNext = NULL;
	
	if (NULL == (*ppHashtable) || (*ppHashtable)->nBucket_size <= 0 || NULL == (*ppHashtable)->hash_func)
	{
		printf("HashFree: func param error\n");
		return -1; 
	}

	pthread_mutex_lock(&(*ppHashtable)->hash_mutex);
	for (i=0; i<(*ppHashtable)->nBucket_size; i++ ) 
	{
		
		for (pItem=(*ppHashtable)->ppItem[i]; pItem!=NULL; pItem=pNext) 
		{
			pNext = pItem->pNext;
			FREEIF(pItem->pKey);
			
			if (free_func)
			{
				free_func(pItem->pMatch_msg);
			}
			else
			{
				FREEIF(pItem->pMatch_msg);	
			}

			FREEIF(pItem);
		}
	}
	pthread_mutex_unlock(&(*ppHashtable)->hash_mutex);

	FREEIF((*ppHashtable)->ppItem);
	pthread_mutex_destroy(&(*ppHashtable)->hash_mutex);
	FREEIF((*ppHashtable));
	return 0;
}

//Function purpose: insert an item in the hash table
//Input parameters: pointer to hash table pointer, key pointer to hash table, numeric pointer to hash table
//Output parameters: None
//Return value: Insert successfully, return TRUE, insert failed, return FALSE
//Remarks: Because the last node did not check whether the keys are the same, modify it as follows
int HashInsert(StHash_Table **ppHashtable, char *pKey, void *pData)
{
	unsigned int nIdx = 0;
	StHash_Item *pItem = NULL;
	int nLen = 0;
	
	if (NULL == (*ppHashtable) || (*ppHashtable)->nBucket_size <= 0 || NULL == (*ppHashtable)->hash_func || NULL == pKey)
	{	
		printf("HashInsert: func param error\n");
		return -1;
	}

	pthread_mutex_lock(&(*ppHashtable)->hash_mutex);
	nIdx = (*ppHashtable)->hash_func(pKey);
	nIdx %= (*ppHashtable)->nBucket_size;
	
	/* if index is free, fill it */
	if (NULL == (*ppHashtable)->ppItem[nIdx]) 
	{ 
		(*ppHashtable)->ppItem[nIdx] = (StHash_Item *)malloc(sizeof(StHash_Item));
		if (NULL == (*ppHashtable)->ppItem[nIdx])
		{
			printf("HashInsert: Call MM_MALLOC_WITH_DESC error\n");
			pthread_mutex_unlock(&(*ppHashtable)->hash_mutex);
			return -1;
		}

		nLen = strlen((char *)pKey);
		(*ppHashtable)->ppItem[nIdx]->pKey  = (char *)malloc(nLen + 1);
		if (NULL == (*ppHashtable)->ppItem[nIdx]->pKey)
		{
			printf("HashInsert: Call MM_MALLOC_WITH_DESC error\n");
			pthread_mutex_unlock(&(*ppHashtable)->hash_mutex);
			return -1;
		}

		memset((*ppHashtable)->ppItem[nIdx]->pKey, 0, nLen+1);
		memcpy((*ppHashtable)->ppItem[nIdx]->pKey, pKey, nLen);
		
		(*ppHashtable)->ppItem[nIdx]->pMatch_msg = pData;
		//printf("key=[%s] value=[%s]\n", (*ppHashtable)->ppItem[nIdx]->pKey, (char *)(*ppHashtable)->ppItem[nIdx]->pMatch_msg);
		(*ppHashtable)->ppItem[nIdx]->pNext = NULL;
		(*ppHashtable)->nHashtable_len++;
		pthread_mutex_unlock(&(*ppHashtable)->hash_mutex);
		return 0;
	}

	
	/* no, then find if the key already exists, and reach the last link */
	for (pItem=(*ppHashtable)->ppItem[nIdx]; pItem->pNext != NULL; pItem = pItem->pNext)
	{
		if (0 == strcmp(pKey, pItem->pKey))
		{	
			FREEIF(pItem->pMatch_msg);
			pItem->pMatch_msg = pData;
			pthread_mutex_unlock(&(*ppHashtable)->hash_mutex);
			return 0; 
		}
	}

	if (0 == strcmp(pKey, pItem->pKey))
	{
		FREEIF(pItem->pMatch_msg);
		pItem->pMatch_msg = pData;
		pthread_mutex_unlock(&(*ppHashtable)->hash_mutex);
		return 0; 
	}
	
	pItem->pNext = (StHash_Item *)malloc(sizeof(StHash_Item));
	if (NULL == pItem->pNext)
	{
		printf("HashInsert: Call malloc error\n");
		pthread_mutex_unlock(&(*ppHashtable)->hash_mutex);
		return -1;  
	}

	nLen = strlen(pKey);
	pItem->pNext->pKey  = (char *)malloc(nLen + 1);
	if (NULL == pItem->pNext->pKey)
	{
		printf("HashInsert: Call MM_MALLOC_WITH_DESC error\n");
		pthread_mutex_unlock(&(*ppHashtable)->hash_mutex);
		return -1;
	}
    memset(pItem->pNext->pKey, 0,nLen + 1);
	memcpy(pItem->pNext->pKey, pKey, nLen);
	
	pItem->pNext->pMatch_msg = pData;
	pItem->pNext->pNext = NULL;
	(*ppHashtable)->nHashtable_len++;
	pthread_mutex_unlock(&(*ppHashtable)->hash_mutex);
	return 0;
}

//Function purpose: delete an item in the hash table
//Input parameters: pointer to hash table pointer, pointer to hash table key
//Output parameters: None
//Return value: delete successfully, return the deleted numeric address, delete failed, return NULL
void *HashDel(StHash_Table **ppHashtable, char *pKey)
{
	unsigned int nIdx = 0;
	void *pData;
	StHash_Item *pItem = NULL; 
	StHash_Item *pLast_item = NULL;
	
	if (NULL == (*ppHashtable) || (*ppHashtable)->nBucket_size <= 0 || NULL == (*ppHashtable)->hash_func || NULL == pKey)
	{
		printf("HashDel: func param error\n");
		return NULL; 
	}

	pthread_mutex_lock(&(*ppHashtable)->hash_mutex);
	nIdx = (*ppHashtable)->hash_func(pKey);
	nIdx %= (*ppHashtable)->nBucket_size;

	for (pItem=(*ppHashtable)->ppItem[nIdx]; pItem!=NULL; pItem=pItem->pNext) 
	{
		//strcmp comparison function failed
		if (NULL != pItem->pKey)
		{
			if (0 == strcmp(pKey, pItem->pKey)) 
			{
				/* fix list */
				if (pLast_item)
				{
					pLast_item->pNext = pItem->pNext;
				}
				else /* is first item */
				{
					(*ppHashtable)->ppItem[nIdx] = pItem->pNext;
				}

				FREEIF(pItem->pKey);
				pData = pItem->pMatch_msg;	
				FREEIF(pItem);
				(*ppHashtable)->nHashtable_len--;
				pthread_mutex_unlock(&(*ppHashtable)->hash_mutex);
				return pData;
				
			}		
		}
		
		
		pLast_item = pItem;
	} 

	pthread_mutex_unlock(&(*ppHashtable)->hash_mutex);
	return NULL;
}


//Function purpose: Get the value of the corresponding hash table according to the key of the hash table
//Input parameters: pointer to hash table, key pointer to hash table
//Output parameters: None
//Return value: Get successful, return the obtained numeric address, get failed, return NULL
void *HashData(StHash_Table *pHashtable, char *pKey)
{
	unsigned int nIdx = 0;
	StHash_Item *pItem = NULL;
	
	if (NULL == pHashtable || pHashtable->nBucket_size <= 0 || NULL == pHashtable->hash_func || NULL == pKey)
	{
		printf("HashData: func Param error\n");
		return NULL; 
	}

	pthread_mutex_lock(&pHashtable->hash_mutex);		  
	nIdx = pHashtable->hash_func(pKey);   
	nIdx %= pHashtable->nBucket_size;
	for (pItem = pHashtable->ppItem[nIdx]; pItem != NULL; pItem = pItem->pNext )
	{
		if (strcmp(pKey, pItem->pKey) == 0)
		{
			pthread_mutex_unlock(&pHashtable->hash_mutex);
			return pItem->pMatch_msg;
		}
	}

	pthread_mutex_unlock(&pHashtable->hash_mutex);
	return NULL;
}



//Function purpose: Determine whether the hash table is empty
//Input parameter: pointer to hash table
//Output parameters: None
//Return value: If empty, return TRUE, if not empty, return FALSE
int IsHashEmpty(StHash_Table *pHashtable)
{
	pthread_mutex_lock(&pHashtable->hash_mutex);
	int nHash_length = pHashtable->nHashtable_len;
	pthread_mutex_unlock(&pHashtable->hash_mutex);
	
	if (0 == nHash_length)
	{
		return 1;	
	}
	else
	{
		return 0;
	}
}

//Function purpose: Determine the hash table length
//Input parameter: pointer to hash table
//Output parameters: nHash_length
//Return value: size of hash table
int HashtableLen(StHash_Table *pHashtable)
{
	
	pthread_mutex_lock(&pHashtable->hash_mutex);
	int nHash_length = pHashtable->nHashtable_len;
	pthread_mutex_unlock(&pHashtable->hash_mutex);
	return nHash_length;
}

