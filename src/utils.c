#include "include/utils.h"

// get time_t value of a date supporting a DD-MM-YYYY HH-MM format
int date_to_secs(char* the_date, char* the_hour_min, time_t *ret_val){
    struct tm date_time = {0,0,0,0,0,0,0,0,0};
    char *parser, delim[] = "-:", date[DATE_LEN], hour_min[TIME_LEN];
    if(strlen(the_date) != DATE_LEN-1 || strlen(the_hour_min) != TIME_LEN-1)
        return -1;

    strcpy(date, the_date); strcpy(hour_min, the_hour_min);
    // get day,month and year of date
    parser = strtok(date, delim); date_time.tm_mday = atoi(parser); 
    parser = strtok(NULL, delim); date_time.tm_mon = atoi(parser)-1;
    parser = strtok(NULL, delim); date_time.tm_year = atoi(parser) - 1900;
    parser = strtok(hour_min, delim); date_time.tm_hour = atoi(parser); 
    parser = strtok(NULL, delim); date_time.tm_min = atoi(parser); 
    
    *ret_val = mktime(&date_time);
    return 0;
}

// print a usage message and exit with code 1(FAILURE)
void usage_error(){
    fprintf(stdout, "Usage: bitcoin -a bitCoinBalancesFile -t transactionFile -v bitCoinValue "\
    "-h1 senderHT_Buckets -h2 recvHT_Buckets -b bucketSize\nArguments 1 and 2 should be valid paths"\
    " and arguments 3-6 should be positive integers.\nAll six arguments are mandatory\n");
    exit(EXIT_FAILURE);
}

// print error indicated by errno and exit with code 1(FAILURE)
void perror_exit(char* message){
    perror(message);
    exit(EXIT_FAILURE);
}

// print a message in stderr and exit with code 1(FAILURE)
void error_exit(const char* format,...){
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

// print a message in stderr and exit with code 1(FAILURE)
int error_return(int ret_val, const char* format,...){
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fprintf(stdout, "\033[0m");   //reset text color
    return ret_val;
}

// print command syntax details
int syntax(uint8_t code){
    switch (code)
    {
        case 0:
            fprintf(stdout, "requestTransaction senderWalletID receiverWalletID amount [date] [time]\n");
            break;
        case 1:
            fprintf(stdout, "requestTransactions senderID receiverID amount [date] [time];senderID2 receiverID2 amount_2 [date_2] [time_2];"\
            " … ;senderIDn receiverID_n amount_n [date_n] [time_n];\nor\nrequestTransactions inputFile\n");
            break;
        case 2:
            fprintf(stdout, "findEarnings walletID [date1][time1][date2][time2]\n");
            break;
        case 3:
            fprintf(stdout, "findPayments walletID [date1][time1][date2][time2]\n");
            break;
        case 4:
            fprintf(stdout, "walletStatus walletID\n");
            break;
        case 5:
            fprintf(stdout, "bitCoinStatus bitCoinID\n");
            break;
        case 6:
            fprintf(stdout, "traceCoin bitCoinID\n");
            break;
        case 7:
            fprintf(stdout, "printHT\n");
            break;
        default:
            return error_return(-1, "syntax: Error, command with code %hhd does not exist, type 'help' to get a command list\n", code);
    }
    return 0;
}

// print each valid command 
void help(){
    uint8_t code;
    fprintf(stdout, "bitcoin, version 1.0.0\nType 'help' to see this list\n\ncommand list\n------------\n");
    for(code = 0; code < COMMANDS; code++)
        syntax(code);
}

// given a list of pointers to transactions, add up the value of each transaction that was made during a certain time range
int get_money(struct G_list* transPtrList, time_t start, time_t end){
    struct Transaction *trPtr;
    struct G_node *parser;
    time_t trans_time;
    int money = 0;
    if(transPtrList == NULL)
        return error_return(-1, "find: NULL pointer to transaction pointers list\n");
    if(start >= end)
        return error_return(-2, "find: invalid time range, start_time = %s and end_time = %s", ctime(&start), ctime(&end));
    for(parser = transPtrList->head; parser != NULL; parser = parser->next){
        trPtr = *((struct Transaction**)parser->data);
        date_to_secs(trPtr->date, trPtr->time, &trans_time);

        // check time range
        if(start <= trans_time && end >= trans_time)
            money += trPtr->value;
    }
    return money;
}

// find Earnings/Payments: estimate the amount of money a user earned or payed within a time range(if no time is given print full history)
int find(const char *stream, struct HashTable* table, uint8_t code){
    uint8_t argc;
    char id[MAX_ID], start_date[DATE_LEN], end_date[DATE_LEN], start_time[TIME_LEN], end_time[TIME_LEN];
    time_t start, end;
    struct Wallet *wallet;
    argc = sscanf(stream, "%s%s%s%s%s", id, start_date, start_time, end_date, end_time);
    
    // if one argument was given there is no time range limit so 'start' gets 0 and 'end' gets the current moment of time
    if(argc == 1){
        start = 0;
        end = time(NULL);
        if(code != 0 && code != 1)
            return error_return(-1, "find: Error, invalid code %hhd", code);
    }
    // if dates and times were passed convert them to time_t in order to simplify time comparisons 
    else if(argc == 5){
        if(date_to_secs(start_date, start_time, &start) == -1 || date_to_secs(end_date, end_time, &end) == -1)
            return error_return(-1, "find: Error, invalid date format; the appropriate is DD-MM-YY MM:HH\n");

        // set code to indicate that date and time was specified(code = 2 or 3) and it either a sender(code = 2) or a receiver(code = 3) type query
        if(code == 0)
            code = 2;
        else if(code == 1)
            code = 3;
        else
            return error_return(-2, "find: Error, invalid code %hhd", code);
    }
    // else the command's format was invalid
    else{
        fprintf(stdout, "find(Earnings/Payments): Error, invalid syntax\nTry this:\t");
        syntax(2 + code);
        return -1;
    }

    // get the user's wallet from the hash table
    if((wallet = htSearch(table, id)) == NULL)
        return error_return(-3, "find: user with id equal to '%s' is not a registered sender or/and receiver\n", id);
    
    // if total transactions as a receiver is to be printed, map transPrint to each transaction pointer in the send_list, user has in his wallet
    if(code == 0)
        listMap(&(wallet->recv_list), transPtrPrint);

    // else if total transactions as a sender is to be printed, map transPrint to each transaction pointer in the recv_list, user has in his wallet
    else if(code == 1)
        listMap(&(wallet->send_list), transPtrPrint);

    // else if total EARNINGS is to be estimated
    else if(code == 2)
        fprintf(stdout, "findEarnings: %s earned %d$\n", wallet->userID, get_money(&(wallet->recv_list), start, end));
    
    // else code has been set to 3(see above if statement), so total PAYMENTS is to be estimated
    else
        fprintf(stdout, "findPayments: %s payed %d$\n", wallet->userID , get_money(&(wallet->send_list), start, end));
    return code;
}
