// struct Transaction holds all necessary information about a bitcoin transaction
#ifndef trans_H_
#define trans_H_
#include "wallet.h"
#include "define.h"
#include "hashTable.h"

/*struct Transaction Declaration and Interface*/
struct Transaction{
    char transID[TRANS_ID];
    char senderID[MAX_ID];
    char receiverID[MAX_ID];
    int value;
    char date[DATE_LEN];
    char time[TIME_LEN];
};

// linked list's of transaction POINTERS member methods
int transPtrAssign(void*, const void*);
int transPtrPrint(void*);

// linked list's of transactions member methods
void* transCompare(void*, const void*);
int transAssign(void*, const void*);
int transPrint(void*);
void transFree(void*);
struct HashTable;
int requestTransaction(const char*, struct G_list*, struct G_list*, struct HashTable*, struct HashTable*, uint8_t, uint8_t);
#endif