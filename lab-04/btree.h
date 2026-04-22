#pragma once

#include <stdbool.h>
#include <stdio.h>

#define T_DEGREE 2                  // Минимальная степень дерева
#define MAX_KEYS (2 * T_DEGREE - 1) // Максимальное количество ключей
#define MAX_CHILDREN (2 * T_DEGREE) // Максимальное количество детей
#define MAX_KEY_LEN 7               // 6 букв и терминальный нуль

typedef struct BTreeNode
{
    char keys[MAX_KEYS][MAX_KEY_LEN];
    double values[MAX_KEYS];
    struct BTreeNode *children[MAX_CHILDREN];
    struct BTreeNode *parent;
    int count;
    bool is_leaf;
} BTreeNode;

typedef struct BTree
{
    BTreeNode *root;
} BTree;

void copyKey(char *dest, const char *src);
void btreeInsert(BTree *tree, const char *key, double value);
void btreeDelete(BTree *tree, const char *key);
void btreePrintToStream(BTreeNode *node, int level, FILE *out);
double *findValue(BTreeNode *node, const char *key);
void freeTree(BTreeNode *node);