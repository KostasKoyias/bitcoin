#ifndef define_H_
#define define_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include "../gen-list/list.h"
#define MAX(a,b) ((a) > (b)) ? (a) : (b)
#define MIN(a,b) ((a) < (b)) ? (a) : (b)
#define COMMANDS 7
#define DATE_LEN 11
#define TIME_LEN 6
#define MAX_ID 50
#define TRANS_ID 15
#define AMOUNT 10
#define LINE 512
#define BUCKS 5
#define MAX_COMMAND 22

#endif