#pragma once

#include <stdbool.h>

typedef struct Node
{
    int value;
    struct Node *first_child;
    struct Node *last_child;
    struct Node *next;
    struct Node *prev;
    struct Node *parent;
} Node;

Node *CreateNode(int value);

Node *AddChild(Node *parent, int value);

void FreeMemory(Node *root);

bool DeleteSubtree(Node **root, Node *target);
