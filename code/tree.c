#include "tree.h"

/*  search for the latest transaction of a bitcoin that involves a given user, if there is no such transaction NULL is returned
    whenever a transaction is made if there is still some money left in the sender's part of the coin, of the two nodes created
    he is represented by the right one*/ 
struct Node* searchTree(struct Node* root, const char* userID){
    struct Node* result;

    // return NULL if tree is "empty"
    if(root == NULL)
        return NULL;

    // if this node represents a transaction containing the given user AS A SENDER(!)
    if(strcmp(root->userID, userID) == 0){
        // check the right branch, because of the convention mentioned above(after a transaction sender gets the right branch)
        if(root->right != NULL && (result = searchTree(root->right, userID)))
            return result;
        return root;
    }
    
    // check whether he receives a part of this bit coin down the tree(left branch)
    return searchTree(root->left, userID);
}

/* split(not always) a node into two(sometimes just one) other(s), return the number of nodes generated*/
int splitNode(struct Node* node, struct Transaction** transactionPtr){
    struct Transaction* transaction = *transactionPtr;
    int rem;

    if(node == NULL || transaction == NULL)
        return 0;

    node->trans = transaction;
    rem = MAX(0, transaction->value - node->value);
    
    // insert transaction, receiver first
    if((node->left = malloc(sizeof(struct Node))) == NULL){
        perror("malloc");
        return -1;
    }
    strcpy(node->left->userID, transaction->receiverID); node->left->value = MIN(node->value, transaction->value);
    node->left->trans = NULL, node->left->right = node->left->left = NULL;

    // check whether this was a 'perfect' transfer or is there a remainder, if so create a node for the sender because he still owns a part
    if(node->value > transaction->value){
        if((node->right = malloc(sizeof(struct Node))) == NULL){
            perror("malloc");
            return -1;
        }
        strcpy(node->right->userID, transaction->senderID); node->right->value = node->value - transaction->value;
        node->right->trans = NULL, node->right->right = node->right->left = NULL;
        return 2;
    }
    return 1;
}

/* print all transactions in the tree in a DFS way, return -1 if an invalid pointer is passed */
int printTree(const struct Node* root){
    int ret_left = -1, ret_right = -1;
    if(root == NULL)
        return -1;
    if(root->trans != NULL)
        transPrint(root->trans);
    if(root->left != NULL)
        ret_left = (printTree(root->left) == 0);
    if(root->right != NULL)
        ret_right = (printTree(root->right) == 0);
    return (ret_right+1) || (ret_left+1);
}

/* free all nodes of a binary tree recursively(DFS-Type), given it's root*/
int freeTree(struct Node* node){
    if(node == NULL)
        return -1;
    if(node->right != NULL){
        freeTree(node->right);
        free(node->right);
    }
    if(node->left != NULL){
        freeTree(node->left);
        free(node->left);
    }
    return 0;
}

/* assign a node pointer A(*node) to another node pointer B(*other_node), given a pointer to A(node) and a pointer to B(other_node)  */
int nodePtrAssign(void *node, const void *other_node){
    struct Node **ptr = (struct Node**)node, **other_ptr = (struct Node**)other_node;
    *ptr = *other_ptr;
}

/* given a node representing a bitcoin's part estimate the number of transactions this part has been involved in*/
int nodeTrans(const struct Node *root){
    if(root == NULL)
        return -1;
    // no left branch means no receiver, so the coin was not passed
    if(root->left == NULL)
        return 0;
    // left branch, but not a right one means it was completely passed to another user, 
    // so return 1(for this one) plus the number of times the receiver passed it
    else if(root->right == NULL)
        return 1 + nodeTrans(root->left);
    // else the coin was split so return 1(for this one), plus the number of times it's two parts were later passed
    else  
        return 1 + nodeTrans(root->right) + nodeTrans(root->left);
}



