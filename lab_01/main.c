#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tree.h"
#include "tree_service.h"

// Функция для очистки буфера ввода
void ClearInputBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

// Добавление узла
void MenuAddNode(Node **root)
{
    int value, parent_value;

    if (*root == NULL)
    {
        printf("Дерево пустое. Введите значение для корня: ");
        if (scanf("%d", &value) != 1)
        {
            printf("Ошибка ввода.\n");
            ClearInputBuffer();
            return;
        }

        *root = CreateNode(value);
        if (!(*root))
        {
            printf("Ошибка: не удалось создать корень.\n");
            return;
        }

        printf("Корень создан.\n");
    }
    else
    {
        printf("Введите значение родителя: ");
        if (scanf("%d", &parent_value) != 1)
        {
            ClearInputBuffer();
            return;
        }
        printf("Введите значение нового узла: ");
        if (scanf("%d", &value) != 1)
        {
            ClearInputBuffer();
            return;
        }

        if (AddChildByValue(*root, parent_value, value))
        {
            printf("Узел успешно добавлен.\n");
        }
        else
        {
            printf("Ошибка: родитель не найден или значение уже существует.\n");
        }
    }
}

// Печать дерева
void MenuPrintTree(const Node *root)
{
    if (!root)
    {
        printf("Дерево пустое.\n");
        return;
    }
    printf("\n----- СТРУКТУРА ДЕРЕВА -----\n");
    PrintTree(root, 0, 0ULL);
    printf("----------------------------\n");
}

// Удаление узла
void MenuDeleteNode(Node **root)
{
    if (!*root)
    {
        printf("Дерево пустое.\n");
        return;
    }
    int value;
    printf("Введите значение узла для удаления (вместе с поддеревом): ");
    if (scanf("%d", &value) != 1)
    {
        ClearInputBuffer();
        return;
    }

    if (DeleteSubtreeByValue(root, value))
    {
        printf("Узел (вместе с поддеревом) успешно удален.\n");
    }
    else
    {
        printf("Узел с таким значением не найден.\n");
    }
}

// 4. Получить степень дерева
void MenuGetDegree(const Node *root)
{
    if (!root)
    {
        printf("Дерево пустое, степень 0.\n");
        return;
    }
    printf("Степень дерева: %u\n", GetDegree(root));
}

int main()
{
    Node *root = NULL;
    int choice = -1;

    printf("Добро пожаловать в программу работы с деревом!\n");

    while (choice != 0)
    {
        printf("\n------ ТЕКСТОВОЕ МЕНЮ ------\n");
        printf("1. Добавить узел\n");
        printf("2. Визуализировать дерево\n");
        printf("3. Удалить узел (поддерево)\n");
        printf("4. Определить степень дерева\n");
        printf("0. Выход\n");
        printf("----------------------------\n");
        printf("Выберите действие: ");

        if (scanf("%d", &choice) != 1)
        {
            printf("Ошибка: введите число!\n");
            ClearInputBuffer();
            continue;
        }

        switch (choice)
        {
        case 1:
            MenuAddNode(&root);
            break;
        case 2:
            MenuPrintTree(root);
            break;
        case 3:
            MenuDeleteNode(&root);
            break;
        case 4:
            MenuGetDegree(root);
            break;
        case 0:
            printf("Завершение работы...\n");
            DeleteSubtree(&root, root);
            break;
        default:
            printf("Неверный пункт меню.\n");
        }
    }

    return 0;
}