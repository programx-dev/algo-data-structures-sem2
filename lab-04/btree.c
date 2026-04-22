#include "btree.h"
#include <stdlib.h>
#include <string.h>

// Безопасное копирование с учетом ограничения длины
void copyKey(char *dest, const char *src)
{
    strncpy(dest, src, MAX_KEY_LEN - 1);
    dest[MAX_KEY_LEN - 1] = '\0';
}

BTreeNode *createNode(bool is_leaf)
{
    BTreeNode *new_node = calloc(1, sizeof(*new_node));

    if (new_node == NULL)
    {
        return NULL;
    }
    new_node->is_leaf = is_leaf;

    return new_node;
}

/*
Находится узел в дереве содержащий ключ и индекс по которому лежит либо -1 если ключ не найден, и
узел на котором поиск остановился
*/
BTreeNode *findNodeWithKey(BTreeNode *root, const char *key, int *out_index) {
    if (out_index != NULL) {
        *out_index = -1;
    }
    if (root == NULL) {
        return NULL;
    }
    BTreeNode *curr = root;
    while (curr != NULL) {
        int i = 0;
        while (i < curr->count && strcmp(key, curr->keys[i]) > 0) {
            i++;
        }
        if (i < curr->count && strcmp(key, curr->keys[i]) == 0) {
            if (out_index != NULL) {
                *out_index = i;
            }
            return curr;
        }

        if (curr->is_leaf) {
            return curr;
        }

        curr = curr->children[i];
    }

    return NULL;
}

/*
Поскольку максимальное значение узлов в задании равно 4, и они упорядочены,
то достаточно реализовать примитивный линейный поиск, вместо бинарного.
Возвращаем указатеоль чтобы была возможно передать NULL если не было найдено ключа,
если он был найден то по указателю можно посмотреть значение
*/
double *findValue(BTreeNode *node, const char *key)
{
    int index = -1;
    BTreeNode *found_node = findNodeWithKey(node, key, &index);

    if (found_node != NULL && index != -1)
    {
        return &(found_node->values[index]);
    }

    return NULL;
}

// Вставка в узел где имеется гарантированное свободное место
void insertIntoNode(BTreeNode *node, const char *key, double value, BTreeNode *new_child)
{
    int i = node->count - 1;

    while (i >= 0 && strcmp(key, node->keys[i]) < 0)
    {
        copyKey(node->keys[i + 1], node->keys[i]);
        node->values[i + 1] = node->values[i];
        node->children[i + 2] = node->children[i + 1];
        i--;
    }

    copyKey(node->keys[i + 1], key);
    node->values[i + 1] = value;
    node->children[i + 2] = new_child;

    if (new_child != NULL)
    {
        new_child->parent = node;
    }

    node->count++;
}

// Главная функция для вставки ключа, при необходимости выполняет разбиение дерева
void splitAndInsert(BTree *tree, BTreeNode *node, const char *key, double value, BTreeNode *new_child)
{
    // В узле есть свободнео место
    if (node->count < MAX_KEYS)
    {
        insertIntoNode(node, key, value, new_child);
        return;
    }

    // Узел переполнен - разделяем его на два узла
    BTreeNode *new_node = createNode(node->is_leaf);
    if (new_node == NULL)
    {
        return;
    }
    new_node->parent = node->parent;

    // Индекс медианы
    int mid = T_DEGREE - 1;

    char mid_key[MAX_KEY_LEN];
    copyKey(mid_key, node->keys[mid]);
    double mid_val = node->values[mid];

    // Переносим правую половину ключей и детей в новый узел
    int right_i = 0;
    for (int i = mid + 1; i < MAX_KEYS; i++)
    {
        copyKey(new_node->keys[right_i], node->keys[i]);
        new_node->values[right_i] = node->values[i];
        new_node->children[right_i] = node->children[i];

        if (new_node->children[right_i] != NULL)
        {
            new_node->children[right_i]->parent = new_node;
        }
        right_i++;
    }

    // Отдельно связываем самого правого ребенка вне цикла т.к. кло-во ключей + 1 = кол-во детей
    new_node->children[right_i] = node->children[MAX_KEYS];
    if (new_node->children[right_i] != NULL)
    {
        new_node->children[right_i]->parent = new_node;
    }

    node->count = mid;
    new_node->count = right_i;

    // Вставляем новый ключ в нужную половину
    if (strcmp(key, mid_key) < 0)
    {
        insertIntoNode(node, key, value, new_child);
    }
    else
    {
        insertIntoNode(new_node, key, value, new_child);
    }

    // ПРикрепляем медиану к верхнему узлу

    // Случай если придется создать новый корень
    if (node->parent == NULL)
    {
        BTreeNode *new_root = createNode(false);
        if (new_root == NULL)
        {
            return;
        }
        copyKey(new_root->keys[0], mid_key);
        new_root->values[0] = mid_val;
        new_root->children[0] = node;
        new_root->children[1] = new_node;
        new_root->count = 1;

        node->parent = new_root;
        new_node->parent = new_root;
        tree->root = new_root;
    }
    // Случай поднять к уже существующему узлу
    else
    {
        splitAndInsert(tree, node->parent, mid_key, mid_val, new_node);
    }
}

/*
Интерфейс для вставки, отдельно обрабатывает случай когда дерево пустое
*/
void btreeInsert(BTree *tree, const char *key, double value)
{
    if (tree->root == NULL)
    {
        tree->root = createNode(true);
        if (tree->root != NULL)
        {
            copyKey(tree->root->keys[0], key);
            tree->root->values[0] = value;
            tree->root->count = 1;
        }
        return;
    }

    int index = -1;
    BTreeNode *target_node = findNodeWithKey(tree->root, key, &index);

    if (index != -1)
    {
        return;
    }

    if (target_node != NULL)
    {
        splitAndInsert(tree, target_node, key, value, NULL);
    }
}

// Ищет ключ в заданном узле (линейный поиск)
int getChildIndex(BTreeNode *parent, BTreeNode *child)
{
    for (int i = 0; i <= parent->count; i++)
    {
        if (parent->children[i] == child)
        {
            return i;
        }
    }
    return -1;
}

/*
Исправить дерево при удалении узла - объединения или перемещенеи клчючей
*/
void repairTree(BTree *tree, BTreeNode *node)
{
    // Проверяем корень
    if (node->parent == NULL)
    {
        if (node->count == 0)
        {
            if (node->children[0] != NULL)
            {
                BTreeNode *old = node;
                tree->root = node->children[0];
                tree->root->parent = NULL;
                free(old);
            }
            else
            {
                free(node);
                tree->root = NULL;
            }
        }
        return;
    }

    if (node->count >= T_DEGREE - 1)
    {
        return;
    }

    BTreeNode *parent = node->parent;
    int idx = getChildIndex(parent, node);

    // Пробуем занять у левого соседа
    if (idx > 0 && parent->children[idx - 1]->count > T_DEGREE - 1)
    {
        BTreeNode *left = parent->children[idx - 1];

        for (int i = node->count; i > 0; i--)
        {
            copyKey(node->keys[i], node->keys[i - 1]);
            node->values[i] = node->values[i - 1];
            node->children[i + 1] = node->children[i];
        }
        node->children[1] = node->children[0];

        copyKey(node->keys[0], parent->keys[idx - 1]);
        node->values[0] = parent->values[idx - 1];
        node->children[0] = left->children[left->count];
        if (node->children[0] != NULL)
        {
            node->children[0]->parent = node;
        }
        node->count++;

        copyKey(parent->keys[idx - 1], left->keys[left->count - 1]);
        parent->values[idx - 1] = left->values[left->count - 1];
        left->count--;
        return;
    }

    // Пробуем занять у прагого соседа
    if (idx < parent->count && parent->children[idx + 1]->count > T_DEGREE - 1)
    {
        BTreeNode *right = parent->children[idx + 1];

        copyKey(node->keys[node->count], parent->keys[idx]);
        node->values[node->count] = parent->values[idx];
        node->children[node->count + 1] = right->children[0];
        if (node->children[node->count + 1] != NULL)
        {
            node->children[node->count + 1]->parent = node;
        }
        node->count++;

        copyKey(parent->keys[idx], right->keys[0]);
        parent->values[idx] = right->values[0];

        for (int i = 0; i < right->count - 1; i++)
        {
            copyKey(right->keys[i], right->keys[i + 1]);
            right->values[i] = right->values[i + 1];
            right->children[i] = right->children[i + 1];
        }
        right->children[right->count - 1] = right->children[right->count];
        right->count--;
        return;
    }

    // Слияние с левым
    if (idx > 0)
    {
        BTreeNode *left = parent->children[idx - 1];

        copyKey(left->keys[left->count], parent->keys[idx - 1]);
        left->values[left->count] = parent->values[idx - 1];
        left->count++;

        for (int i = 0; i < node->count; i++)
        {
            copyKey(left->keys[left->count], node->keys[i]);
            left->values[left->count] = node->values[i];
            left->children[left->count] = node->children[i];
            if (left->children[left->count] != NULL)
            {
                left->children[left->count]->parent = left;
            }
            left->count++;
        }
        left->children[left->count] = node->children[node->count];
        if (left->children[left->count] != NULL)
        {
            left->children[left->count]->parent = left;
        }

        for (int i = idx - 1; i < parent->count - 1; i++)
        {
            copyKey(parent->keys[i], parent->keys[i + 1]);
            parent->values[i] = parent->values[i + 1];
            parent->children[i + 1] = parent->children[i + 2];
        }
        parent->count--;
        free(node);
        repairTree(tree, parent);
    }
    // Слияние с правым (если левго нет)
    else
    {
        BTreeNode *right = parent->children[idx + 1];

        copyKey(node->keys[node->count], parent->keys[idx]);
        node->values[node->count] = parent->values[idx];
        node->count++;

        for (int i = 0; i < right->count; i++)
        {
            copyKey(node->keys[node->count], right->keys[i]);
            node->values[node->count] = right->values[i];
            node->children[node->count] = right->children[i];
            if (node->children[node->count] != NULL)
            {
                node->children[node->count]->parent = node;
            }
            node->count++;
        }
        node->children[node->count] = right->children[right->count];
        if (node->children[node->count] != NULL)
        {
            node->children[node->count]->parent = node;
        }

        for (int i = idx; i < parent->count - 1; i++)
        {
            copyKey(parent->keys[i], parent->keys[i + 1]);
            parent->values[i] = parent->values[i + 1];
            parent->children[i + 1] = parent->children[i + 2];
        }
        parent->count--;
        free(right);
        repairTree(tree, parent);
    }
}

void btreeDelete(BTree *tree, const char *key)
{
    BTreeNode *curr = tree->root;
    while (curr != NULL)
    {
        int i = 0;
        while (i < curr->count && strcmp(key, curr->keys[i]) > 0)
        {
            i++;
        }

        if (i < curr->count && strcmp(key, curr->keys[i]) == 0)
        {
            if (curr->is_leaf)
            {
                for (int j = i; j < curr->count - 1; j++)
                {
                    copyKey(curr->keys[j], curr->keys[j + 1]);
                    curr->values[j] = curr->values[j + 1];
                }
                curr->count--;
                repairTree(tree, curr);
                return;
            }
            else
            {
                BTreeNode *succ = curr->children[i + 1];
                while (!succ->is_leaf)
                {
                    succ = succ->children[0];
                }

                char succ_key[MAX_KEY_LEN];
                copyKey(succ_key, succ->keys[0]);
                double succ_val = succ->values[0];

                copyKey(curr->keys[i], succ_key);
                curr->values[i] = succ_val;

                for (int j = 0; j < succ->count - 1; j++)
                {
                    copyKey(succ->keys[j], succ->keys[j + 1]);
                    succ->values[j] = succ->values[j + 1];
                }
                succ->count--;
                repairTree(tree, succ);
                return;
            }
        }

        if (curr->is_leaf)
        {
            break;
        }
        curr = curr->children[i];
    }
}

void btreePrintToStream(BTreeNode *node, int level, FILE *out)
{
    if (node == NULL)
    {
        return;
    }
    for (int i = 0; i < level; i++)
    {
        fprintf(out, "    ");
    }
    fprintf(out, "[");
    for (int i = 0; i < node->count; i++)
    {
        fprintf(out, "%s:%.2f%s", node->keys[i], node->values[i], (i < node->count - 1 ? ", " : ""));
    }
    fprintf(out, "]\n");

    if (!node->is_leaf)
    {
        for (int i = 0; i <= node->count; i++)
        {
            btreePrintToStream(node->children[i], level + 1, out);
        }
    }
}

void freeTree(BTreeNode *node)
{
    if (node == NULL)
    {
        return;
    }

    if (!node->is_leaf)
    {
        for (int i = 0; i <= node->count; i++)
        {
            freeTree(node->children[i]);
        }
    }
    free(node);
}