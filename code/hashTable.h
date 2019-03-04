/* a simple hash table implementation, managing collisions by creating overflow blocks*\
\* each block contains records of type "struct Wallet*", so just pointers             */

#ifndef hashTable_H_
#define hashTable_H_
#include "wallet.h"
#include "define.h"

// a block contains 'capacity' walletPointers
struct Block{
    struct Wallet *(*wallets);
    int empty;
    int capacity;
};

// a hash table contains a number of buckets and a linked list of blocks for each of them
struct HashTable{
    struct G_list* blocks; // array of linked lists of blocks, one list for each bucket
    uint8_t buckets; // number of buckets
    char *name;
    int capacity; // bucket/block capacity
    int (*hashFunction)(const char*, const int);
};

// hash table manipulation methods
int UniversalHashing(const char*, const int);
int htInit(struct HashTable*, char*, const uint8_t, const int, int (*)(const char*, const int));
int htFree(struct HashTable*);
int htInsert(struct HashTable*, struct Wallet**);
void htPrint(const struct HashTable*);
struct Wallet* htSearch(struct HashTable*, const char*);

// 'member methods' of linked list of type 'struct Block'
void* blockComp(void*, const void*);
int blockAssign(void*, const void*);
void blockFree(void*);
int blockPrint(void*);

#endif

