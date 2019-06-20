#include "include/wallet.h"

/**************************************************************\
 The following are 'member methods' of a linked list of wallets
\**************************************************************/

/* compare two wallets, based on their 'userID' field*/
void* walletCompare(void* walletA, const void* userID){
    if(strcmp(((struct Wallet*)walletA)->userID, (char*)userID) == 0)
        return walletA;
    else
        return NULL;
}

/* assign a wallet's content to another*/
int walletAssign(void *wallet, const void* userID){
    struct Wallet *wall = (struct Wallet*)wallet;
    strcpy(wall->userID, (char*)userID);
    wall->balance = 0;

    if(wallet == NULL || userID == NULL)
        return -1;

    // initialize transactions list
    wall->send_list = wall->recv_list = (struct G_list){.head = NULL, .type_size = sizeof(struct Transaction*), .length = 0,\
    .comp = NULL, .assign = transPtrAssign, .print = transPtrPrint, .free_data = NULL, .value = NULL};

    // initialize the tree_node_pointers list
    wall->quota_list = (struct G_list){.head = NULL, .type_size = sizeof(struct Node*), .length = 0, .comp = NULL,\
    .assign = nodePtrAssign, .print = NULL, .free_data = NULL, .value = NULL};

    return 0;
}

/* free a heap-allocated wallet*/
void walletFree(void* wallet){
    struct Wallet* wall = (struct Wallet*)wallet;
    listFree(&(wall->send_list));
    listFree(&(wall->recv_list));
    listFree(&(wall->quota_list));
    free(wall);
}

/* print a wallet's content*/
int walletPrint(void *wallet_void){
    struct Wallet* wallet = (struct Wallet*)wallet_void;
    if(wallet == NULL || wallet->userID == NULL)
        return -1;
    fprintf(stdout, "\n\e[1;4;95mWallet\e[0m\nUserID: ");
    fprintf(stdout,"\e[1;32m%s\e[0m\n", wallet->userID);// print userID in green
    fprintf(stdout, "Balance: %d\n\e[1;4mSender List\e[0m\n", wallet->balance);
    listPrint(&(wallet->send_list));
    fprintf(stdout, "\e[1;4mReceiver List\e[0m\n");
    listPrint(&(wallet->recv_list));
    return 0;
}

