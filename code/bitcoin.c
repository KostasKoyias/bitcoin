/***************************************************************************************************************************************\ 
 This application implements a user-friendly interface for a bitcoin transactions system, using a variety of data structures implemented 
 as generally as possible, eliminating data duplication in order to give quick and efficient access to all data the user might ask for.
 Author: Kostas Koyias https://github.com/KostasKoyias
\***************************************************************************************************************************************/

#include "define.h"
#include "hashTable.h"
#include "wallet.h"
#include "tree.h"
#include "coin.h"
#include "list.h"
#include "utils.h"
#include "trans.h"

int main(int argc, char *argv[]){
    /* declare variables */
    uint16_t bucketSize = 0, bitCoinValue = 0, coinID;
    uint8_t sendBuckets = 0, recvBuckets = 0, i, bucketCapacity;
    FILE *transFile = NULL, *balancesFile = NULL;
    char *line, *buffer, userID[MAX_ID], command[MAX_COMMAND], args[LINE], str, delim[] = ";", file[LINE], ch, *start;
    struct Transaction transaction;
    struct HashTable sendHT, recvHT;
    struct Wallet *wallet, *sendWallet, *recvWallet;
    struct Coin *coinPtr;
    struct Node *nodePtr;
    struct G_list coinlist = {NULL, sizeof(struct Coin), 0, coinCompare, coinAssign, coinPrint, coinFree, NULL}, 
    walletlist = {NULL, sizeof(struct Wallet), 0, walletCompare, walletAssign, walletPrint, walletFree, NULL},
    translist = {NULL, sizeof(struct Transaction), 0, transCompare, transAssign, transPrint, NULL, NULL};

    /* get command line arguments and make sure the usage is correct*/
    for(i = 1; i < argc-1; i++){
        if(strcmp(argv[i], "-a") == 0){
            if((balancesFile = fopen(argv[i+1], "r")) == NULL)
                perror_exit(argv[i+1]);
        }
        else if(strcmp(argv[i], "-t") == 0){
            if((transFile = fopen(argv[i+1], "r")) == NULL)
                perror_exit(argv[i+1]);
        }
        else if(strcmp(argv[i], "-h1") == 0)
            sendBuckets = atoi(argv[i+1]);
        else if(strcmp(argv[i], "-h2") == 0)
            recvBuckets = atoi(argv[i+1]);
        else if(strcmp(argv[i], "-v") == 0)
            bitCoinValue = atoi(argv[i+1]);
        else if(strcmp(argv[i], "-b") == 0)
            bucketSize = atoi(argv[i+1]);
    }
    if(balancesFile == NULL)
        usage_error();
    if(transFile == NULL){
        fclose(balancesFile);
        usage_error();
    }
    if(bitCoinValue <= 0 || bucketSize <=0 || sendBuckets <= 0 || recvBuckets <= 0){
        fclose(balancesFile);
        fclose(transFile);
        usage_error();
    }

    // estimate how many records fit in a bucket and create the sender and the receiver Hash Tables 
    bucketCapacity =  (int)(bucketSize - 2*sizeof(int)) / sizeof(struct Wallet*);
    if((htInit(&sendHT, "Senders", sendBuckets, bucketCapacity, UniversalHashing) == -1) || (htInit(&recvHT, "Receivers", recvBuckets, bucketCapacity, UniversalHashing)))
        goto free;

    /* get bit_coin balances from the input file and update the corresponding data structures */
    while(fscanf(balancesFile, "%s", userID) > 0){
        // create a wallet for the new user
        listInsert(&walletlist, userID);
        while(fscanf(balancesFile, "%hd", &coinID) > 0){

            // for each coin of the user, insert(push) it into the coinlist 
            if(listInsert(&coinlist, &coinID) == -1){
                fprintf(stderr, "Coin %hd exists, abort now...\n", coinID);
                goto free;
            }

            // initialize all field of coin structure with cmd data(e.g bitCoinValue)
            coinPtr = (struct Coin*)(coinlist.head->data);
            strcpy(coinPtr->root.userID, userID); 
            coinPtr->root.value = bitCoinValue;
            nodePtr = &(coinPtr->root);

            // insert(push) a pointer to the quota_list of the user, pointing to the node representing the part of the coin the user owns  
            wallet = (struct Wallet*)(walletlist.head->data);
            listInsert(&(wallet->quota_list), &nodePtr);
            wallet->balance += bitCoinValue;
        }
    }
    if(balancesFile != NULL)
        fclose(balancesFile);

    /* get all initial transactions and update the corresponding data structures */
    while(fgets(args, LINE, transFile) != NULL)
        // add transaction to the transactions list and then add a pointer to it, in the wallet and the corresponding tree of the coinlist
        requestTransaction(args, &translist, &walletlist, &sendHT, &recvHT, 1);
    if(transFile != NULL) 
        fclose(transFile);

    // print current state of wallets and hash tables
    fprintf(stdout, "After reading the input files, we have the following wallets and content for each of them\n");
    listPrint(&walletlist);

    fprintf(stdout, "After reading the input files, Hash Tables look like this\n");
    htPrint(&sendHT); htPrint(&recvHT);

    // prompt user to enter a query until user types 'exit'
    while(1){
        fprintf(stdout, ">> ");

        // get command from cmd
        fscanf(stdin, "%s", command);
        fgets(args, LINE, stdin);

        // a single transaction was requested
        if(strcmp(command, "requestTransaction") == 0)
            requestTransaction(args, &translist, &walletlist, &sendHT, &recvHT, 0);

        // a series of transactions seperated by semi-colon(;) were requested 
        else if(strcmp(command, "requestTransactions") == 0){
            // get first argument
            sscanf(args, "%s", file);

            // if it is a valid file path store it's content into 'args' else args was given from stdin
            if((transFile = fopen(file, "r")) != NULL){
                fprintf(stdout, "gettin transactions from %s ...\n", file);
                fgets(args, LINE, transFile);
                fclose(transFile);
            }

            // for each transaction seperated by semi-colon(delim), attempt to carry it out
            for(i = 0; i < strlen(args); i += strlen(file) + 1){
                if(sscanf(args + i, "%512[^;]s", file) == 1)
                    requestTransaction(file, &translist, &walletlist, &sendHT, &recvHT, 0);
            }
        }
        // estimate the amount of money a user got from bit coin parts within a time range
        else if(strcmp(command, "findEarnings") == 0)
            find(args, &recvHT, 0);

        // estimate the amount of money a user spent by passing a 'piece' of his bit coins to another user
        else if(strcmp(command, "findPayments") == 0)
            find(args, &sendHT, 1);

        // print the amount of money a user has right now, given his userID
        else if(strcmp(command, "walletStatus") == 0){
            if(sscanf(args, "%s", userID) == 1){
                if((wallet = (struct Wallet*)listSearch(&walletlist, userID)) == NULL)
                    fprintf(stdout, "walletStatus: no wallet found for user '%s'\n", userID);
                else
                    fprintf(stdout, "user's %s status is %d$\n", wallet->userID, wallet->balance);
            }
        }
        // given a coin_id return it's initial value, the number of times it was partitioned and it's unspent part in $
        else if(strcmp(command, "bitCoinStatus") == 0){
            // get the coinID from cmd
            sscanf(args, "%hd", &coinID);
            if((coinPtr = (struct Coin*)listSearch(&coinlist, &coinID)) == NULL){
                fprintf(stdout, "There is no bitcoin with code %hd\n", coinID);
                continue;
            }
            coinStatus(coinPtr);
        }
        // print all information about each and every transaction a coin was involved in
        else if(strcmp(command, "traceCoin") == 0){
            sscanf(args, "%hd", &coinID);
            if((coinPtr = (struct Coin*)listSearch(&coinlist, &coinID)) == NULL){
                fprintf(stdout, "There is no bitcoin with code %hd\n", coinID);
                continue;
            }    
            coinPrint(coinPtr);   
        }
        else if(strcmp(command, "exit") == 0)
            break;
        else if(strcmp(command, "help") == 0)
            help();
        else
            fprintf(stderr, "%s: Command not found\nType 'help' to get a list of valid commands\n", command);
        strcpy(args, "\0");

    }

free:    
    /* de-allocate memory, letting go of all resources used in the application */
    listFree(&coinlist);
    listFree(&walletlist);
    listFree(&translist);
    htFree(&sendHT); 
    htFree(&recvHT);
    if(fork() == 0)
        execlp("make", "make", "clean", NULL); //clean up the mess
    wait(NULL);
    return 0;
}