#include "trans.h"
#include "utils.h"

/*******************************************************************************\
 The following are 'member methods' of a linked list of POINTERS to transactions
\*******************************************************************************/

/* assign a trans pointer A(*node) to another trans pointer B(*other_node), given a pointer to A(node) and a pointer to B(other_node)  */
int transPtrAssign(void *node, const void *other_node){
    struct Transaction **ptr = (struct Transaction**)node, **other_ptr = (struct Transaction**)other_node;
    *ptr = *other_ptr;
    return 0;
}

/* given a {pointer to a {pointer to a transaction}} print the inner pointer's content(the transaction) nicely*/
int transPtrPrint(void* transPtrPtr_void){
    struct Transaction *transPtr = *((struct Transaction**)transPtrPtr_void);
    transPrint(transPtr);
    return 0;
}

/* free a {pointer to a {pointer to a transaction}}*/
void transPtrFree(void* transPtrPtr_void){
    free((struct Transaction**)transPtrPtr_void);
}

/*******************************************************************\
 The following are 'member methods' of a linked list of transactions
\*******************************************************************/

/* compare two transactions, based on their transactionID*/
void* transCompare(void* transA, const void* transID){
    if(strcmp(((struct Transaction*)transA)->transID, (char*)transID) == 0)
        return transA;
    else
        return NULL;
}

/* assign a transaction instance to another*/
int transAssign(void *transA, const void *transB){
    struct Transaction *trA = (struct Transaction*)transA, *trB = (struct Transaction*)transB;
    strcpy(trA->transID, trB->transID);       strcpy(trA->senderID, trB->senderID); strcpy(trA->receiverID, trB->receiverID); 
    strcpy(trA->date, trB->date);             strcpy(trA->time, trB->time);         trA->value = trB->value;
    return 0;
}

/* print nicely all information of a given transaction */
int transPrint(void *trans_void){
    struct Transaction* trans = (struct Transaction*)trans_void;
    if(trans == NULL)
        return -1;
    fprintf(stdout, "%s\t%s\t%s\t%d\t%s\t%s\n",\
    trans->transID, trans->senderID, trans->receiverID, trans->value, trans->date, trans->time);
    return 0;
}

/* read transfer data from a file, if the transfer is feasible, make the transfer and update all data structure, return 0
   if syntax is wrong return -2, if the transaction is not feasible or the last transfer recorded is more recent return -1*/
int requestTransaction(const char* stream, struct G_list* translist, struct G_list* walletlist, struct HashTable* sendHT, struct HashTable* recvHT, uint8_t init, uint8_t disp){
    static time_t last_transfer = 0;
    static char max_id[MAX_ID] = "0";
    char str_value[AMOUNT];
    struct Transaction trans, *transPtr;
    struct Wallet *send, *recv;
    struct G_node *parser, *parser_next;
    struct Node *node;
    struct tm *now;
    int rem, temp;
    time_t trans_time;
    uint8_t argc;
    char ch;

    // return -1 for incorrect arguments
    if(stream == NULL || translist == NULL || walletlist == NULL)
        return -1;
    if(init == 0)
        fprintf(stdout, "\033[1;36m");   //Set text's color to 'cyan'
    // if it is about a transaction after the initialization, we need to generate a unique transID, let it be the last we got plus 1
    if(init == 0){
        argc = sscanf(stream, "%s%s%s%s%s", trans.senderID, trans.receiverID, str_value, trans.date, trans.time);
        sprintf(max_id,"%d", atoi(max_id) + 1);
        strcpy(trans.transID, max_id);
    }
    // else it should be given in the file
    else{
        argc = sscanf(stream, "%s%s%s%s%s%s", trans.transID, trans.senderID, trans.receiverID, str_value, trans.date, trans.time);

        // check whether the transID given already exists
        if((transPtr = (struct Transaction*)listSearch(translist, trans.transID)) != NULL)
            return error_return(-2, "requestTransaction(s): Error, there is already a transaction with id equal to %s\n\n", trans.transID);
        
        // update max until now trans_id, in order to be able to generate unique id's after the initialization by adding 1 to the last id used
        if((temp = atoi(trans.transID)) != 0)
            sprintf(max_id, "%d", MAX(atoi(max_id), temp)); 
        
    }

    // no self-transactions allowed
    if(strcmp(trans.senderID, trans.receiverID) == 0)
        return -3;

    // if date and time is not given use current date and time
    if(argc == 3 + init){ 
        time(&trans_time); // get current time as transfer time
        now = gmtime(&trans_time);
        sprintf(trans.date, "%.2d-%.2d-%.4d", now->tm_mday, now->tm_mon + 1, now->tm_year + 1900);
        sprintf(trans.time, "%.2d:%.2d", now->tm_hour + 2, now->tm_min);
    }
    else if(argc == 5 + init)
        date_to_secs(trans.date, trans.time, &trans_time); // convert date and time given to seconds since 1970
    // if syntax is incorrect, return -2
    else{
        fprintf(stdout, "requestTransaction(s): invalid syntax: \"%s\" \n\n", stream);
        syntax(0);
        syntax(1);
        fprintf(stdout, "\033[0m");   //reset text color        
        return -4;
    }

    // store the time of latest transfer in order to reject any "new" transfers with older dates after initialization
    if(init == 1)
        last_transfer = MAX(last_transfer, trans_time);
    // if last transfer was more recent, this transfer is cancelled
    else if(last_transfer > trans_time)
        return error_return(-5, "requestTransaction(s): Error, %s was rejected because the most recent transfer was at %s\n", trans.transID, ctime(&last_transfer));
    trans.value = atoi(str_value);

    // if sender is not yet 'hashed' in the senders hash table
    if((send = htSearch(sendHT, trans.senderID)) == NULL){
        // then look at wallet's list, if he is not there, return -6 because he does not have a wallet
        if((send = (struct Wallet*)listSearch(walletlist, trans.senderID)) == NULL)
            return error_return(-6, "requestTransaction(s) Error, sender %s does not have a wallet\n\n", trans.senderID);
        // else add a pointer to his wallet into the senders hash table
        htInsert(sendHT, &send);
    }
    // do the same thing for the receiver
    if((recv = htSearch(recvHT, trans.receiverID)) == NULL){
        if((recv = (struct Wallet*)listSearch(walletlist, trans.receiverID))== NULL)
            return error_return(-6, "requestTransaction(s): Error, receiver %s does not have a wallet\n\n", trans.receiverID);
        htInsert(recvHT, &recv);
    }

    // check whether the transfer is feasible
    if(send->balance < trans.value)
        return error_return(-7, "requestTransaction(s): Error, transaction %s not feasible for sender %s, balance is %d but %d is requested\n\n",\
        trans.transID, trans.senderID, send->balance, trans.value);
 
    // update the 'global' transaction list and get a pointer to the new transaction
    listInsert(translist, &trans);
    transPtr = (struct Transaction*)(translist->head->data);

    // add this pointer to the transaction pointer list of the two participants
    listInsert(&(send->send_list), &transPtr);
    listInsert(&(recv->recv_list), &transPtr);
    
    // for each part of coin the sender owns, which can be found by accesing the coin_tree_node pointer list in his wallet
    // the receiver gets either the full coin or a part of it(if it's value is larger than the remainder)
    for(parser = send->quota_list.head, rem = trans.value; rem != 0 && parser != NULL; parser = parser_next){
        node = (*((struct Node**)parser->data));
        parser_next = parser->next;

        //estimate the part of the coin to be transferred and update balances for both participants 
        temp = rem;
        if(node == NULL)
            continue;
        rem = MAX(0, rem - node->value);
        send->balance -= temp - rem;
        recv->balance += temp - rem;

        // split(or link) node, add a pointer to the transaction that got the node to split, splitNode returns the number of nodes generated(0,1 or 2)
        temp = splitNode(node, &transPtr, temp);

        // delete this coin part(leaf node) from the leaf_node pointer list of the sender 
        if(parser->next != NULL)
            parser->next->prev = parser->prev;
        if(parser->prev != NULL)
            parser->prev->next = parser->next;
        else
            send->quota_list.head = parser->next; 
        free(parser->data);
        free(parser);

        // add pointers to the coin_node_tree pointers list, in each wallet, remember that by convention sender gets the right branch and receiver the left
        if(temp == 2)
            listInsert(&(send->quota_list), &(node->right));
        listInsert(&(recv->quota_list), &(node->left));
    }

    fprintf(stdout,"requestTransaction(s): completed\n");
    transPrint(&trans);
    fprintf(stdout, "\033[0m");   //reset text color
    if(init == 0 && disp == 1){
        fprintf(stdout, "\t\t\t\bshow current state?(y/n): ");
        while(((ch = getchar()) != 'y') && (ch != 'n'))
            fprintf(stdout, "(y/n): \n");
        if(ch == 'y'){
            htPrint(sendHT);
            htPrint(recvHT);
            putchar('\n');
        }
    }
    last_transfer = MAX(last_transfer, trans_time);
    return 0;
}
