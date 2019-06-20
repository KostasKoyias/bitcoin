#ifndef coin_H_
#define coin_H_
#include "tree.h"
#include "define.h"

/*Coin's Declaration and Interface*/
struct Coin{
    uint16_t coinID;
    struct Node root;
};

/* doubly linked list's of coins 'member methods' */
void* coinCompare(void*, const void*);
int coinAssign(void*, const void*);
int coinPrint(void*);
void coinFree(void*);

// coin manipulation methods implementing queries like 'bitCoinStatus' and 'traceCoin'
int coinUnspent(const struct Coin*);
int coinTimesPassed(const struct Coin*);
int coinStatus(const struct Coin*);

#endif