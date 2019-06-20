#include "include/coin.h"

/************************************************************\
 The following are 'member methods' of a linked list of coins
\************************************************************/

/* search for a coin, given a coinID*/
void* coinCompare(void* node, const void* coinID){
    if(((struct Coin*)node)->coinID == *((uint16_t*)coinID))
        return node;
    else
        return NULL;
}

/* print all information concerning a certain coin*/
int coinPrint(void* data){
    if(data == NULL)
        return -1;
    struct Coin* coin = (struct Coin*)data;
    printf("Coin: %hd\n", coin->coinID);
    if(coin->root.userID == NULL)
        return -1;
    printTree(&(coin->root)); 
    return 0;   
}

/* assign a value to a node representing a bitCoin */
int coinAssign(void* node, const void* coinID){
    struct Coin *coin = (struct Coin*)node;
    
    coin->coinID = *((uint16_t*) coinID);
    coin->root = (struct Node){.userID = "", .value = 0, .trans = NULL, .left = NULL, .right = NULL};
    return 0;
}

/* free a coin node*/
void coinFree(void* coin){
    struct Coin* node = (struct Coin*)coin;
    freeTree(&(node->root));
    free(node);
}

/********************************************************************\
 The following methods implement operations on a struct coin instance
\********************************************************************/

/*  estimate the value of the coin's unspent part
    remember that we made a convention that the remaining part for the owner goes to the right subtree*/
int coinUnspent(const struct Coin* coin){
    const struct Node* node = (const struct Node*)(&(coin->root));
    while(node != NULL){
        // if the owner made a transaction, but kept a part, continue 
        if(node->right != NULL){
            node = node->right;
            continue;
        }
        // if this was the final transaction return the remaining value
        else if(node->left == NULL)
            return node->value;
        // else the owner passed on the coin completely
        else 
            return 0; 
    }
    return -1;
}

/*  given a coinPtr, estimate the number of transactions it has been involved in*/
int coinTimesPassed(const struct Coin* coin){
    if(coin == NULL)
        return -1;
    return nodeTrans((const struct Node*)(&(coin->root)));
}

/*  given a coinID, print: i)it's initial value, ii) it's unpent part(part that still belongs to the initial user and 
    has never been involved into a transaction) and iii) the number of transactions it's been involved in*/
int coinStatus(const struct Coin* coin){
    if(coin == NULL)
        return -1;
    fprintf(stdout, "status of coin %hd\n-------------------\nInitial_Value: %d$\nUnspent_Part: %d$\nInvolved in %d transactions\n",\
    coin->coinID, coin->root.value, coinUnspent(coin), coinTimesPassed(coin));
    return 0;
}

