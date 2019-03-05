#include "hashTable.h"

/*  a Universal Hashing Implementation , appropriate for strings*/
int UniversalHashing(const char* str, const int buckets){
    uint8_t h, a = 19, i;
    int p = 1300751;
    h = 0;
    for(i = 0; i < strlen(str); i++)
        h = (h * a + str[i]) % p;
    return h % buckets;
}

/* search for a wallet in a block based on the userID, if found return 0 else return -1*/
void* blockComp(void* blockPtr, const void* userID){
    int i;
    struct Block* block = (struct Block*)blockPtr;
    // iterate on all non-empty records, looking for wallet->userID
    for(i = 0; i < block->empty && block->wallets[i] != NULL && block->wallets[i]->userID != NULL; i++){
        if(strcmp(block->wallets[i]->userID, (char*)userID) == 0)
            return (void*)(&(block->wallets[i]));
    }
    return NULL;
}

/* insert a record(wallet pointer) into a block*/
int blockAssign(void *blockVoid, const void* blockB_Void){
    int i;
    struct Block *block = (struct Block*)blockVoid, *blockB = (struct Block*)blockB_Void;
    block->capacity = blockB->capacity;

    // get block->capacity wallet pointers
    if((block->wallets = malloc(sizeof(struct Wallet*) * block->capacity)) == NULL)
        return -1;
    
    // initialize them to NULL
    for(i = 1; i < block->capacity; i++)
        block->wallets[i] = NULL;
    
    // insert a record(wallet pointer) in the first empty position(index 0)
    block->wallets[0] = *(blockB->wallets);
    block->empty = 1;
    return 0;
}

/* free all space a block needs, meaning it's wallets vector */
void blockFree(void* block){
    free(((struct Block*)block)->wallets);
    free((struct Block*)block);
}

/* print all wallets in a block */
int blockPrint(void* blockPtr){
    int i;
    struct Block* block = (struct Block*)blockPtr;

    // print header with blue color
    fprintf(stdout, "\033[1;34mBlock\n-----\n\033[0mRecords: %d", block->empty);  
    for(i = 0; i < block->empty; i++)
        walletPrint(block->wallets[i]);
    return 0;
}

/* initialize an empty hash table of wallet pointers*/
int htInit(struct HashTable* table, char *name, const uint8_t buckets, const int blockCapacity, int (*hashFunction)(const char*, const int)){
    uint8_t i;

    table->name = strdup(name);
    table->hashFunction = hashFunction;

    // get 'bucksNumber' buckets for the Hash Table
    if((table->blocks = malloc(sizeof(struct G_list) * buckets)) == NULL){
        perror("htInit - malloc");
        return -1;
    }
    table->buckets = buckets;

    // for each bucket
    for(i = 0; i < buckets; i++)
        // initialize the bucket's block list
        table->blocks[i] = (struct G_list){NULL, sizeof(struct Block), 0, blockComp, blockAssign, blockPrint, blockFree, NULL};
    table->capacity = blockCapacity;
    return 0;
}

/* free up all space needed for a bucket(main plus overflow blocks)*/
int htFree(struct HashTable* table){
    uint8_t b;
    if(table == NULL)
        return -1;

    if(table->name != NULL)
        free(table->name);
    // for each bucket of the hash table, let free it's block list
    for(b = 0; b < table->buckets; b++)
        listFree(&(table->blocks[b]));
    // free all buckets of the hash table
    free(table->blocks);
    return 0;
}

/* insert a user(sender or receiver) in the hash table using a UniversalHashing function on his userID*/ 
int htInsert(struct HashTable* table, struct Wallet** walletPtr){
    uint8_t buck;
    struct Block newBlock = {walletPtr, 0, table->capacity};
    struct G_node *head;
    struct Block *block;

    // find the appropriate bucket and insert the record
    buck = table->hashFunction((*walletPtr)->userID, table->buckets);
    head = table->blocks[buck].head;

    // if first block is full, create a new one and insert the record there
    if(head == NULL || (((struct Block*)head->data)->empty) == table->capacity)
        listInsert(&(table->blocks[buck]), &newBlock);
    // else insert the record at this block and increment the counter by 1
    else{
        block = (struct Block*)head->data;
        block->wallets[block->empty] = *walletPtr;
        block->empty++;
    }
    return 0;
}

/* print hash table in a nice way*/
void htPrint(const struct HashTable* table){
    uint8_t i;

    fprintf(stdout, "\033[1;33m");   //Set the text to the color yellow
    for(i = 0; i < strlen(table->name) + 13; i++)
        putchar('-');
    fprintf(stdout, "\n|HashTable: %s|\n", table->name);
    for(i = 0; i < strlen(table->name) + 13; i++)
        putchar('-');
    putchar('\n');
    fprintf(stdout, "\033[0m"); //Resets the text to default color
    fprintf(stdout, "Buckets: %d\nBlock Capacity: %d\n", table->buckets, table->capacity);
    for(i = 0; i < table->buckets; i++){
        fprintf(stdout, "\033[1;31m _________\n|Bucket: %d|\n|_________|\n\033[0m", i);
        listPrint(&(table->blocks[i]));
    }
}

/* search for a wallet in the hash table*/
struct Wallet* htSearch(struct HashTable* table, const char* userID){
    uint8_t bucket;
    struct Wallet** walletPtrPtr;

    if(table == NULL || userID == NULL)
        return NULL;

    // get the bucket, where this user's wallet might be in
    bucket = UniversalHashing(userID, table->buckets);

    // get a pointer to the 'wallet pointer' that points to the user's wallet or NULL there is no such pointer
    if((walletPtrPtr = (struct Wallet**)listSearch(&(table->blocks[bucket]), userID)) == NULL)
        return NULL;
    else
        return *walletPtrPtr;
}