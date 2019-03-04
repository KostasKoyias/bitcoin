/* Each user has a wallet, storing his id, his balance($) and three lists of pointers. First pointer lists is used to acces the transaction's info
   for each transaction he was involved in as a user and the second for those he had the role of the receiver; the third list is used to access
   the bitcoin parts he know owns, being able to use them in future transactions*/
    
#ifndef wallet_H_
#define wallet_H_
#include <stdint.h>
#include "tree.h"
#include "trans.h"
#include "define.h"

/*Wallet Declaration and Interface*/
struct Wallet{
    char userID[MAX_ID];
    int balance;
    struct G_list send_list;
    struct G_list recv_list;
    struct G_list quota_list;
};

/* the following declare all 'member methods' of a linked list of wallets*/
void* walletCompare(void*, const void*);
int walletAssign(void*, const void*);
int walletPrint(void *);
void walletFree(void*);

#endif