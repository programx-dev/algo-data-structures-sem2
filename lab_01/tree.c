#include "tree.h"

#include <stdlib.h>
#include <stdbool.h>

Node *CreateNode(int value)
{
    Node *new_node = malloc(sizeof(*new_node));
    if (!new_node)
    {
        return NULL;
    }
    new_node->value = value;
    new_node->first_child = NULL;
    new_node->last_child = NULL;
    new_node->next = NULL;
    new_node->prev = NULL;
    new_node->parent = NULL;

    return new_node;
}

Node *AddChild(Node *parent, int value)
{
    if (!parent)
    {
        return NULL;
    }
    Node *new_node = CreateNode(value);
    if (!new_node)
    {
        return NULL;
    }
    new_node->parent = parent;

    if (!parent->first_child)
    {
        parent->first_child = new_node;
        parent->last_child = new_node;
    }
    else
    {
        parent->last_child->next = new_node;
        new_node->prev = parent->last_child;
        parent->last_child = new_node;
    }

    return new_node;
}

void FreeMemory(Node *root)
{
    if (!root)
    {
        return;
    }

    Node *child = root->first_child;
    while (child)
    {
        Node *next_child = child->next;
        FreeMemory(child);
        child = next_child;
    }

    free(root);
}

bool DeleteSubtree(Node **root, Node *target)
{
    if (!root || !(*root) || !target)
    {
        return false;
    }

    bool is_root = (target == *root);

    if (!is_root)
    {
        if (target->prev)
        {
            target->prev->next = target->next;
        }
        else
        {
            target->parent->first_child = target->next;
        }

        if (target->next)
        {
            target->next->prev = target->prev;
        }
        else
        {
            target->parent->last_child = target->prev;
        }
    }
    else
    {
        *root = NULL;
    }

    target->parent = NULL;
    target->next = NULL;
    target->prev = NULL;

    FreeMemory(target);

    return true;
}
