#include "tree_service.h"
#include <stdlib.h>
#include <stdio.h>

unsigned int GetDegree(const Node *target)
{
    if (!target)
        return 0;

    unsigned int max_degree = 0;

    unsigned int count = 0;
    for (const Node *child = target->first_child; child; child = child->next)
    {
        count++;
        unsigned int child_degree = GetDegree(child);
        if (child_degree > max_degree)
        {
            max_degree = child_degree;
        }
    }

    if (count > max_degree)
    {
        max_degree = count;
    }

    return max_degree;
}

Node *FindByValue(Node *root, int value)
{
    if (!root)
    {
        return NULL;
    }

    if (root->value == value)
    {
        return root;
    }

    for (Node *child = root->first_child; child; child = child->next)
    {
        Node *found = FindByValue(child, value);
        if (found)
        {
            return found;
        }
    }
    return NULL;
}

Node *AddChildByValue(Node *root, int parent_value, int value)
{

    if (!root)
    {
        return NULL;
    }

    Node *parent = FindByValue(root, parent_value);

    if (!parent)
    {
        return NULL;
    }

    if (FindByValue(root, value))
    {
        return NULL;
    }

    Node *new_node = AddChild(parent, value);

    return new_node;
}

bool DeleteSubtreeByValue(Node **root, int value)
{
    Node *target = FindByValue(*root, value);

    return DeleteSubtree(root, target);
}

void PrintTree(const Node *root, int depth, unsigned long long int mask)
{
    if (!root)
    {
        return;
    }

    for (int i = 0; i < depth - 1; i++)
    {
        if (i < 64)
        {
            if (mask & (1ULL << i))
            {
                printf("    ");
            }
            else
            {
                printf("|   ");
            }
        }
        else
        {
            printf("|   ");
        }
    }

    if (depth > 0)
    {
        if ((depth - 1) < 64)
        {

            if (mask & (1ULL << (depth - 1)))
            {
                printf("+-- ");
            }
            else
            {
                printf("|-- ");
            }
        }
        else
        {
            printf("|-- ");
        }
    }
    printf("%d\n", root->value);

    const Node *child = root->first_child;
    while (child)
    {
        unsigned long long int new_mask = mask;
        if (depth < 64)
        {
            if (!child->next)
            {
                new_mask |= (1ULL << depth);
            }
        }
        PrintTree(child, depth + 1, new_mask);
        child = child->next;
    }
}
