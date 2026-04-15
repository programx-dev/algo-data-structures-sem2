#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum
{
    TOK_OP,
    TOK_UNARY_MINUS,
    TOK_VAR,
    TOK_NUM,
    TOK_LBRACKET,
    TOK_RBRACKET,
} TokenType;

typedef struct TokenNode
{
    TokenType type;
    char str[16];
    struct TokenNode *next;
} TokenNode;

// Обертка для списка
typedef struct
{
    TokenNode *head;
    TokenNode *tail;
} TokenList;

TokenNode *createNode(TokenType type, const char *val)
{
    TokenNode *node = malloc(sizeof(*node));

    if (!node)
        return NULL;

    node->type = type;
    node->next = NULL;

    if (val)
        strncpy(node->str, val, 15);
    else
        node->str[0] = '\0';

    return node;
}

TokenNode *copyNode(TokenNode *src)
{
    if (!src)
        return NULL;

    return createNode(src->type, src->str);
}

void appendToList(TokenList *list, TokenNode *node)
{
    if (!node)
        return;

    if (!list->head)
    {
        list->head = list->tail = node;
    }
    else
    {
        list->tail->next = node;
        list->tail = node;
    }
}

void freeTokenList(TokenNode *node)
{
    while (node)
    {
        TokenNode *tmp = node->next;
        free(node);
        node = tmp;
    }
}

TokenNode *tokenizer(char *in, int len)
{
    TokenList list = {NULL, NULL};
    int lastType = -1;
    int i = 0;

    while (i < len && in[i] != '\0')
    {
        if (isspace(in[i]))
        {
            i++;
            continue;
        }

        if (in[i] == '(')
        {
            appendToList(&list, createNode(TOK_LBRACKET, "("));
            lastType = TOK_LBRACKET;
            i++;
        }
        else if (in[i] == ')')
        {
            appendToList(&list, createNode(TOK_RBRACKET, ")"));
            lastType = TOK_RBRACKET;
            i++;
        }
        else if (strchr("+-*/", in[i]))
        {
            char op[2] = {in[i], '\0'};

            TokenType type;
            if (in[i] == '-' && (lastType == -1 || lastType == TOK_LBRACKET))
            {
                type = TOK_UNARY_MINUS;
            }
            else
            {
                type = TOK_OP;
            }

            appendToList(&list, createNode(type, op));
            lastType = type;
            i++;
        }
        else if (isdigit(in[i]))
        {
            char buff[16] = {0};
            int k = 0;
            while (i < len && (isdigit(in[i]) || in[i] == '.'))
            {
                if (k < 15)
                    buff[k++] = in[i++];
                else
                    i++;
            }
            appendToList(&list, createNode(TOK_NUM, buff));
            lastType = TOK_NUM;
        }
        else if (isalpha(in[i]))
        {
            char buff[16] = {0};
            int k = 0;
            while (i < len && isalnum(in[i]))
            {
                if (k < 15)
                    buff[k++] = in[i++];
                else
                    i++;
            }
            appendToList(&list, createNode(TOK_VAR, buff));
            lastType = TOK_VAR;
        }
        else
            i++;
    }
    return list.head;
}

// СТЕК ДЛЯ ОПЕРАТОРОВ
typedef struct StackNode
{
    TokenNode *token;
    struct StackNode *next;
} StackNode;

void push(StackNode **top, TokenNode *token)
{
    StackNode *node = malloc(sizeof(*node));

    if (!node)
        return;

    node->token = token;
    node->next = *top;
    *top = node;
}

TokenNode *pop(StackNode **top)
{
    if (!(*top))
        return NULL;

    StackNode *tmp = *top;
    TokenNode *token = tmp->token;
    *top = tmp->next;

    free(tmp);

    return token;
}

TokenNode *peek(StackNode *top)
{
    return top ? top->token : NULL;
}

int getPriority(TokenNode *node)
{
    if (!node)
    {
        return -1;
    }
    if (node->type == TOK_UNARY_MINUS)
    {
        return 4;
    }
    if (node->type == TOK_OP)
    {
        char op = node->str[0];
        if (op == '*' || op == '/')
        {
            return 3;
        }
        if (op == '+' || op == '-')
        {
            return 2;
        }
    }
    if (node->type == TOK_LBRACKET)
    {
        return 0;
    }
    return -1;
}

TokenNode *toPostfix(TokenNode *in)
{
    StackNode *stack = NULL;
    TokenList out = {NULL, NULL};
    TokenNode *curr = in;

    while (curr)
    {
        if (curr->type == TOK_NUM || curr->type == TOK_VAR)
        {
            appendToList(&out, copyNode(curr));
        }
        else if (curr->type == TOK_LBRACKET)
        {
            push(&stack, curr);
        }
        else if (curr->type == TOK_RBRACKET)
        {
            while (peek(stack) && peek(stack)->type != TOK_LBRACKET)
            {
                appendToList(&out, copyNode(pop(&stack)));
            }
            pop(&stack);
        }
        else
        {
            int p_curr = getPriority(curr);
            while (peek(stack) && getPriority(peek(stack)) >= p_curr)
            {
                // Доделать унырный
                appendToList(&out, copyNode(pop(&stack)));
            }
            push(&stack, curr);
        }
        curr = curr->next;
    }

    while (peek(stack))
    {
        appendToList(&out, copyNode(pop(&stack)));
    }

    return out.head;
}

int main()
{
    return 0;
}