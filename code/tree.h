/*Each coin has a field named node, being the root of a binary tree keeping track of the coin's history by traversing the tree, we can get
  all information about the transactions it's been involved in by following the 'struct Transaction' pointers stored in the internal nodes.*/

#ifndef node_H_
#define node_H_
#include "trans.h"
#include "define.h"
#include "utils.h"

/*BitCoin's Binary Tree of Transactions History Declaration and Interface*/
struct Node{
    char userID[MAX_ID];
    int value;
    struct Transaction *trans;
    struct Node *left;
    struct Node *right;
};

// node/tree manipulation
int freeTree(struct Node*);
struct Node* searchTree(struct Node*, const char*);
int splitNode(struct Node* node, struct Transaction**, int);
int printTree(const struct Node*);
int nodeTrans(const struct Node*);

// methods supporting a node pointer double linked list
int nodePtrAssign(void*, const void*);

#endif