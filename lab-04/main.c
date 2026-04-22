#include "btree.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    const char *input_file = "input.txt";
    const char *out_file = "output.txt";

    FILE *in = fopen(input_file, "r");
    FILE *out = fopen(out_file, "w");

    if (in == NULL || out == NULL)
    {
        fprintf(stderr, "Ошибка: не удалось открыть файлы!\n");
        if (in != NULL)
        {
            fclose(in);
        }
        if (out != NULL)
        {
            fclose(out);
        }
        return 1;
    }

    BTree tree;
    tree.root = NULL;

    int command;
    char key[MAX_KEY_LEN];
    char buffer[256];
    double value;

    while (fscanf(in, "%d", &command) != EOF)
    {
        switch (command)
        {
        case 1:
        {
            // Сначала читаем ключ во временный буфер
            if (fscanf(in, "%255s %lf", buffer, &value) == 2)
            {
                fprintf(out, "Команда: 1 %s %.2f\n", buffer, value);

                if (strlen(buffer) > 6)
                {
                    fprintf(out, "Результат: Ошибка, ключ слишком длинный (> 6 символов)\n");
                }
                else if (findValue(tree.root, buffer) != NULL)
                {
                    fprintf(out, "Результат: Ошибка, ключ '%s' уже существует\n", buffer);
                }
                else
                {
                    copyKey(key, buffer);
                    btreeInsert(&tree, key, value);
                }
            }
            break;
        }
        case 2:
        {
            if (fscanf(in, "%255s", buffer) == 1)
            {
                fprintf(out, "Команда: 2 %s\n", buffer);

                if (strlen(buffer) > 6)
                {
                    fprintf(out, "Результат: Ошибка, ключ слишком длинный (> 6 символов)\n");
                }
                else if (findValue(tree.root, buffer) == NULL)
                {
                    fprintf(out, "Результат: Ошибка, ключ не найден\n");
                }
                else
                {
                    btreeDelete(&tree, buffer);
                }
            }
            break;
        }
        case 3:
        {
            fprintf(out, "Команда: 3 (печать структуры)\n");
            fprintf(out, "Результат:\n");
            if (tree.root == NULL)
            {
                fprintf(out, "    [ Дерево пусто ]\n");
            }
            else
            {
                fprintf(out, "=== ПЕЧАТЬ B-ДЕРЕВА (t=%d) ===\n", T_DEGREE);
                btreePrintToStream(tree.root, 1, out);
            }
            break;
        }
        case 4:
        {
            if (fscanf(in, "%255s", buffer) == 1)
            {
                fprintf(out, "Команда: 4 %s\n", buffer);

                if (strlen(buffer) > 6)
                {
                    fprintf(out, "Результат: Ошибка, ключ слишком длинный (> 6 символов)\n");
                }
                else
                {
                    double *v = findValue(tree.root, buffer);
                    if (v != NULL)
                    {
                        fprintf(out, "Результат: Найдено значение %.2f\n", *v);
                    }
                    else
                    {
                        fprintf(out, "Результат: Ключ не найден\n");
                    }
                }
            }
            break;
        }
        default:
        {
            fprintf(out, "Команда: %d\n", command);
            fprintf(out, "Результат: Неизвестная операция\n");
        }
        }
        fprintf(out, "==================================\n\n");
    }

    freeTree(tree.root);
    fclose(in);
    fclose(out);

    printf("Обработка завершена. Результаты сохранены в '%s'.\n", out_file);

    return 0;
}