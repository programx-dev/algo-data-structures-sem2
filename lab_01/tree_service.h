#pragma once

#include "tree.h"
#include <stdbool.h>

unsigned int GetDegree(const Node *target);

Node *FindByValue(Node *root, int value);

Node *AddChildByValue(Node *root, int parent_value, int value);

bool DeleteSubtreeByValue(Node **root, int value);

void PrintTree(const Node *root, int depth, unsigned long long int mask);