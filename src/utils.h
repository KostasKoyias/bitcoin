#ifndef utils_H_
#define utils_H_
#include "define.h"
#include "hashTable.h"

void usage_error();
void perror_exit(char*);
void error_exit(const char*,...);
int error_return(int, const char*, ...);
void help();
int date_to_secs(char* date, char* time, time_t*);
int syntax(uint8_t);
int get_money(struct G_list*, time_t, time_t);
int find(const char*, struct HashTable*, uint8_t);

#endif