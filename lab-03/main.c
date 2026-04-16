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

typedef struct
{
    TokenNode *head;
    TokenNode *tail;
} TokenList;

TokenNode *createNode(TokenType type, const char *val)
{
    TokenNode *node = malloc(sizeof(*node));
    if (!node)
    {
        return NULL;
    }
    node->type = type;
    node->next = NULL;
    if (val)
    {
        strncpy(node->str, val, 15);
    }
    else
    {
        node->str[0] = '\0';
    }
    return node;
}

TokenNode *copyNode(TokenNode *src)
{
    if (!src)
    {
        return NULL;
    }
    return createNode(src->type, src->str);
}

void appendToList(TokenList *list, TokenNode *node)
{
    if (!node)
    {
        return;
    }
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

void printTokenList(TokenNode *head)
{
    while (head)
    {
        printf("[%s] ", head->str);
        head = head->next;
    }
    printf("\n");
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
        {
            i++;
        }
    }
    return list.head;
}

// СТЕК
typedef struct StackNode
{
    TokenNode *token;
    struct StackNode *next;
} StackNode;

void push(StackNode **top, TokenNode *token)
{
    StackNode *node = malloc(sizeof(*node));
    if (!node)
    {
        return;
    }
    node->token = token;
    node->next = *top;
    *top = node;
}

TokenNode *pop(StackNode **top)
{
    if (!(*top))
    {
        return NULL;
    }
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
    switch (node->type)
    {
    case TOK_UNARY_MINUS:
        return 4;
    case TOK_OP:
        switch (node->str[0])
        {
        case '*':
        case '/':
            return 3;
        case '+':
        case '-':
            return 2;
        default:
            return -1;
        }
    case TOK_LBRACKET:
        return 0;
    default:
        return -1;
    }
}

// ДЕЙКСТРА
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
                if (curr->type == TOK_UNARY_MINUS && peek(stack)->type == TOK_UNARY_MINUS)
                {
                    break;
                }
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

// ДЕРЕВО
typedef enum
{
    ND_OP,
    ND_UNARY,
    ND_VAR,
    ND_NUM
} NodeKind;

typedef struct Node
{
    NodeKind kind;
    union
    {
        char op;
        char name[16];
        double value;
    } data;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct NodeStack
{
    Node *node;
    struct NodeStack *next;
} NodeStack;

Node *createTreeNode(TokenNode *token)
{
    Node *node = malloc(sizeof(*node));
    if (!node)
    {
        return NULL;
    }
    node->left = node->right = NULL;
    switch (token->type)
    {
    case TOK_NUM:
        node->kind = ND_NUM;
        node->data.value = atof(token->str);
        break;
    case TOK_VAR:
        node->kind = ND_VAR;
        strncpy(node->data.name, token->str, 15);
        break;
    case TOK_UNARY_MINUS:
        node->kind = ND_UNARY;
        node->data.op = '-';
        break;
    default:
        node->kind = ND_OP;
        node->data.op = token->str[0];
        break;
    }
    return node;
}

Node *createNodeOp(char op)
{
    Node *node = malloc(sizeof(*node));
    if (node)
    {
        node->kind = ND_OP;
        node->data.op = op;
        node->left = node->right = NULL;
    }

    return node;
}

void pushNode(NodeStack **top, Node *n)
{
    NodeStack *s = malloc(sizeof(*s));
    if (!s)
    {
        return;
    }
    s->node = n;
    s->next = *top;
    *top = s;
}

Node *popNode(NodeStack **top)
{
    if (!(*top))
    {
        return NULL;
    }
    NodeStack *tmp = *top;
    Node *n = tmp->node;
    *top = tmp->next;
    free(tmp);
    return n;
}

void freeTree(Node *root)
{
    if (!root)
    {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    free(root);
}

Node *buildTree(TokenNode *in)
{
    if (!in)
    {
        return NULL;
    }

    NodeStack *stack = NULL;
    TokenNode *curr = in;

    while (curr)
    {
        Node *newNode = createTreeNode(curr);
        if (curr->type == TOK_NUM || curr->type == TOK_VAR)
        {
            pushNode(&stack, newNode);
        }
        else if (curr->type == TOK_UNARY_MINUS)
        {
            newNode->left = popNode(&stack);
            pushNode(&stack, newNode);
        }
        else
        {
            newNode->right = popNode(&stack);
            newNode->left = popNode(&stack);
            pushNode(&stack, newNode);
        }
        curr = curr->next;
    }
    return popNode(&stack);
}

void printTree(Node *root, int level)
{
    if (!root)
    {
        return;
    }
    printTree(root->right, level + 1);
    for (int i = 0; i < level; i++)
    {
        printf("    ");
    }
    if (root->kind == ND_NUM)
    {
        printf("%.2f\n", root->data.value);
    }
    else if (root->kind == ND_VAR)
    {
        printf("%s\n", root->data.name);
    }
    else
    {
        printf("%c\n", root->data.op);
    }
    printTree(root->left, level + 1);
}

Node *copySubTree(Node *root)
{
    if (!root)
    {
        return NULL;
    }

    Node *new_root = malloc(sizeof(*new_root));
    if (!new_root)
    {
        return NULL;
    }

    *new_root = *root;

    new_root->left = copySubTree(root->left);
    new_root->right = copySubTree(root->right);

    return new_root;
}

void makeTransform(Node *root)
{
    if (!root)
        return;

    makeTransform(root->left);
    makeTransform(root->right);

    if (root->kind == ND_OP && (root->data.op == '+' || root->data.op == '-'))
    {
        if (root->left && root->left->kind == ND_OP && root->left->data.op == '/' &&
            root->right && root->right->kind == ND_OP && root->right->data.op == '/')
        {
            char sign = root->data.op;

            Node *old_ldiv = root->left;
            Node *old_rdiv = root->right;

            Node *a = old_ldiv->left;
            Node *b = old_ldiv->right;
            Node *c = old_rdiv->left;
            Node *d = old_rdiv->right;

            free(old_ldiv);
            free(old_rdiv);

            Node *new_denominator = createNodeOp('*');
            new_denominator->left = b;
            new_denominator->right = d;

            Node *ad = createNodeOp('*');
            ad->left = a;
            ad->right = copySubTree(d);

            Node *bc = createNodeOp('*');
            bc->left = copySubTree(b);
            bc->right = c;

            Node *new_numerator = createNodeOp(sign);
            new_numerator->left = ad;
            new_numerator->right = bc;

            root->data.op = '/';
            root->left = new_numerator;
            root->right = new_denominator;
        }
    }
}

int main()
{
    char input[] = "a / (-(-X + Y)) + c / d";
    int len = strlen(input);

    printf("Input: %s\n", input);

    TokenNode *tokens = tokenizer(input, len);
    printf("Tokens: ");
    printTokenList(tokens);

    TokenNode *rpn = toPostfix(tokens);
    printf("RPN:    ");
    printTokenList(rpn);

    Node *root = buildTree(rpn);
    printf("\nTree structure:\n");
    printTree(root, 0);

    makeTransform(root);

    printf("\nTree transformed:\n");
    printTree(root, 0);

    freeTree(root);
    freeTokenList(tokens);
    freeTokenList(rpn);


    return 0;
}