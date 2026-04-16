#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// //////////////////////////////
//      РАБОТА С ТОКЕНТАМИ
// //////////////////////////////

// Перечисление типов токенов
typedef enum
{
    TOK_OP,
    TOK_UNARY_MINUS,
    TOK_VAR,
    TOK_NUM,
    TOK_LBRACKET,
    TOK_RBRACKET,
} TokenType;

// Токен - используется для односвязного списка
typedef struct TokenNode
{
    TokenType type;
    char *str;
    struct TokenNode *next;
} TokenNode;

// Обертка над токенами - хранит указатель на начло и конец односвязного списка
typedef struct
{
    TokenNode *head;
    TokenNode *tail;
} TokenList;

// Создать токен с указанными типом и строкой
TokenNode *createTokenNode(TokenType type, const char *value)
{
    TokenNode *new_node = malloc(sizeof(*new_node));
    if (!new_node)
    {
        return NULL;
    }
    new_node->type = type;
    new_node->next = NULL;

    if (value)
    {
        new_node->str = malloc(strlen(value) + 1);
        if (new_node->str)
        {
            strcpy(new_node->str, value);
        }
        else {
            free(new_node);
            return NULL;
        }
    }
    else
    {
        new_node->str = NULL;
    }
    return new_node;
}

// Создаёт копию токена
TokenNode *copyTokenNode(TokenNode *source)
{
    if (!source)
    {
        return NULL;
    }
    return createTokenNode(source->type, source->str);
}

// Добавляет указанный токен в конец односвязного спсика токенов
void appendToTokenList(TokenList *list, TokenNode *node)
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

// Освободить односвязный список токенов
void freeTokenList(TokenNode *head)
{
    while (head)
    {
        TokenNode *tmp_node = head->next;
        if (head->str)
        {
            free(head->str);
        }
        free(head);
        head = tmp_node;
    }
}

// Распечатать односвязный список токенов
void printTokenList(TokenNode *head)
{
    while (head)
    {
        printf("[%s] ", head->str);
        head = head->next;
    }
    printf("\n");
}

// Преобразовать строку в односвязный список токенов
TokenNode *tokenizer(const char *in)
{
    TokenList token_list = {NULL, NULL};
    int last_token_type = -1;
    int in_len = strlen(in);
    int i = 0;


    while (i < in_len)
    {
        if (isspace(in[i]))
        {
            i++;
            continue;
        }

        // Открывающая скобка
        if (in[i] == '(')
        {   
            TokenNode *tmp = createTokenNode(TOK_LBRACKET, "(");
            if (!tmp) {
                freeTokenList(token_list.head);
                return NULL;
            }
            appendToTokenList(&token_list, tmp);
            last_token_type = TOK_LBRACKET;
            i++;
        }
        // Закрывающая скобка
        else if (in[i] == ')')
        {
            TokenNode *tmp = createTokenNode(TOK_RBRACKET, ")");
            if (!tmp) {
                freeTokenList(token_list.head);
                return NULL;
            }
            appendToTokenList(&token_list, tmp);
            last_token_type = TOK_RBRACKET;
            i++;
        }
        // Знак операции
        else if (strchr("+-*/", in[i]))
        {
            char op_buffer[2] = {in[i], '\0'};
            TokenType current_type;
            // Унарный минус: если знак минус с начала строки или стоит после открывающей скобки
            if (in[i] == '-' && (last_token_type == -1 || last_token_type == TOK_LBRACKET))
            {
                current_type = TOK_UNARY_MINUS;
            }
            else
            {
                current_type = TOK_OP;
            }
            TokenNode *tmp = createTokenNode(current_type, op_buffer);
            if (!tmp) {
                freeTokenList(token_list.head);
                return NULL;
            }
            appendToTokenList(&token_list, tmp);
            last_token_type = current_type;
            i++;
        }
        // Число (огр. 255 знаков)
        else if (isdigit(in[i]))
        {
            char buff[256] = {0};
            int buff_i = 0;
            while (i < in_len && (isdigit(in[i]) || in[i] == '.'))
            {   
                // Считаем первые 255 цифр и точку
                if (buff_i < 255)
                {
                    buff[buff_i++] = in[i++];
                }
                // Оставшиеся просто пропускаем
                else
                {
                    i++;
                }
            }
            TokenNode *tmp = createTokenNode(TOK_NUM, buff);
            if (!tmp) {
                freeTokenList(token_list.head);
                return NULL;
            }
            appendToTokenList(&token_list, tmp);
            last_token_type = TOK_NUM;
        }
        // Переменная (первый символ - буква) (огр. 255 символов)
        else if (isalpha(in[i]))
        {
            char buff[256] = {0};
            int buff_i = 0;
            // После первой буквы могут быть и цифры
            while (i < in_len && isalnum(in[i]))
            {   
                // Читаем первые 255 символов
                if (buff_i < 255)
                {
                    buff[buff_i++] = in[i++];
                }
                // Остальные просто пропускаем
                else
                {
                    i++;
                }
            }
            TokenNode *tmp = createTokenNode(TOK_VAR, buff);
            if (!tmp) {
                freeTokenList(token_list.head);
                return NULL;
            }
            appendToTokenList(&token_list, tmp);
            last_token_type = TOK_VAR;
        }
        else
        {
            i++;
        }
    }
    return token_list.head;
}

// //////////////////////////////////////////////
//      ПРИВЕДЕНИЕ К ОБРАТОНОЙ ПОЛЬСКОЙ ЗАПИСИ
// //////////////////////////////////////////////

// Стек токенов
typedef struct StackNode
{
    TokenNode *token;
    struct StackNode *next;
} StackNode;

// Положить на вершину стека новый токен. Возвращает флаг об успехе операции
bool pushOp(StackNode **top, TokenNode *token)
{
    StackNode *new_node = malloc(sizeof(*new_node));
    if (!new_node)
    {
        return 0;
    }
    new_node->token = token;
    new_node->next = *top;
    *top = new_node;
    return 1;
}

// Достать токен с вершины стека
TokenNode *popOp(StackNode **top)
{
    if (!(*top))
    {
        return NULL;
    }
    StackNode *tmp_node = *top;
    TokenNode *token = tmp_node->token;
    *top = tmp_node->next;
    free(tmp_node);
    return token;
}

// Посмотреть на токен с вершины стека
TokenNode *peekOp(StackNode *top)
{
    return top ? top->token : NULL;
}

// Очистить стек
void freeStackToken(StackNode *top) {
    if (!top) {
        return;
    }
    while (top) {
        popOp(&top);
    }
}

// Получить приоритет оператора - используется в алгоритме Дейкстры
int getOperatorPriority(TokenNode *node)
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
    case TOK_RBRACKET:
        return 1;
    case TOK_LBRACKET:
        return 0;
    default:
        return -1;
    }
}

// Алгоритм Дейкстры - приведение к обратной польской записи
TokenNode *toPostfix(TokenNode *head)
{
    StackNode *op_stack = NULL;
    TokenNode *curr_token = head;
    TokenList postfix_list = {NULL, NULL};

    // Идём по токенам в односвязном списке
    while (curr_token)
    {   
        // Безусловно добавляем число или перемнную в выходной список
        if (curr_token->type == TOK_NUM || curr_token->type == TOK_VAR)
        {   
            TokenNode *tmp = copyTokenNode(curr_token);
            if (!tmp) {
                freeTokenList(postfix_list.head);
                freeStackToken(op_stack);
                return NULL;
            }

            appendToTokenList(&postfix_list, tmp);
        }
        // Если открывающая скобка - безусловно добавлем в стек. Позволяет начать новый отсчет для операторов
        else if (curr_token->type == TOK_LBRACKET)
        {
            if (!pushOp(&op_stack, curr_token)) {
                freeTokenList(postfix_list.head);
                freeStackToken(op_stack);
                return NULL;
            }
        }
        // Если закрывающая скобка - то полностью выталкиваем в итоговый список все токены, до открывающей скобки.
        // Открывающая скобка при этом удаляется из стека, и в список не входит
        else if (curr_token->type == TOK_RBRACKET)
        {
            while (peekOp(op_stack) && peekOp(op_stack)->type != TOK_LBRACKET)
            {   
                TokenNode *tmp = copyTokenNode(popOp(&op_stack));
                if (!tmp) {
                    freeTokenList(postfix_list.head);
                    freeStackToken(op_stack);
                    return NULL;
                }
            
                appendToTokenList(&postfix_list, tmp);
            }
            popOp(&op_stack); // Удаляем открывающую скобку
        }
        // Если оператор
        else
        {
            int curr_priority = getOperatorPriority(curr_token);
            // Для левоассоциативных операторов:
            // Если приоритет текущего оператора > просматриваемого, то добавляем на вершину стека.
            // Если приоритет текущего оператора <= просматриваемого, то вытесняем в итоговый список все операторы с приоритетом
            // Вытеснение операторов с равным приоритетом гарантирует левоассоциативность, т.е. то, оператор идущий раньше в выыражении стоит ближе к операндам
            while (peekOp(op_stack) && getOperatorPriority(peekOp(op_stack)) >= curr_priority)
            {   
                // Для правоассоциативных операторов наоборо - кто позже стоит в выражении должен выполняться раньше
                if (curr_token->type == TOK_UNARY_MINUS && peekOp(op_stack)->type == TOK_UNARY_MINUS)
                {
                    break;
                }
                TokenNode *tmp = copyTokenNode(popOp(&op_stack));
                if (!tmp) {
                    freeTokenList(postfix_list.head);
                    freeStackToken(op_stack);
                    return NULL;
                }

                appendToTokenList(&postfix_list, tmp);
            }
            if (!pushOp(&op_stack, curr_token)) {
                freeTokenList(postfix_list.head);
                freeStackToken(op_stack);
                return NULL;
            }

        }
        curr_token = curr_token->next;
    }
    // Все что осталось в стеке - выталкиваем в итоговый список
    while (peekOp(op_stack))
    {
        appendToTokenList(&postfix_list, copyTokenNode(popOp(&op_stack)));
    }
    return postfix_list.head;
}

// /////////////////////////////
//      РАБОТА С ДЕРЕВОМ
// /////////////////////////////

// Перечисление типов узов дерева
typedef enum
{
    ND_OP,
    ND_UNARY,
    ND_VAR,
    ND_NUM
} NodeType;

// Структура узла дерева
typedef struct Node
{
    NodeType type;
    union
    {
        char op;
        char *var;
        double num;
    } data;
    struct Node *left;
    struct Node *right;
} Node;

// Стек для узлов дерева
typedef struct NodeStack
{
    Node *tree_node;
    struct NodeStack *next;
} NodeStack;

// Очистить дерево
void freeTree(Node *root)
{
    if (!root)
    {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    if (root->type == ND_VAR && root->data.var)
    {
        free(root->data.var);
    }
    free(root);
}

// Создать узел дерева на основе токена
Node *createTreeNode(TokenNode *token)
{
    Node *new_node = malloc(sizeof(*new_node));
    if (!new_node)
    {
        return NULL;
    }
    new_node->left = new_node->right = NULL;
    switch (token->type)
    {
    case TOK_NUM:
    {
        new_node->type = ND_NUM;
        new_node->data.num = atof(token->str);
        break;
    }
    case TOK_VAR:
    {
        new_node->type = ND_VAR;
        new_node->data.var = malloc(strlen(token->str) + 1);
        if (new_node->data.var)
        {
            strcpy(new_node->data.var, token->str);
        }
        else {
            free(new_node);
            return NULL;
        }
        break;
    }
    case TOK_UNARY_MINUS:
    {
        new_node->type = ND_UNARY;
        new_node->data.op = '-';
        break;
    }
    default:
    {
        new_node->type = ND_OP;
        new_node->data.op = token->str[0];
        break;
    }
    }
    return new_node;
}

// Создать узел дерева из символа оператора
Node *createOpNode(char operator_char)
{
    Node *new_node = malloc(sizeof(Node));
    if (new_node)
    {
        new_node->type = ND_OP;
        new_node->data.op = operator_char;
        new_node->left = new_node->right = NULL;
    }
    return new_node;
}

// Рекрсивно создать копию поддерева
Node *copySubTree(Node *root)
{
    if (!root)
    {
        return NULL;
    }
    Node *new_node = malloc(sizeof(*new_node));
    if (!new_node)
    {
        return NULL;
    }
    new_node->left = NULL;
    new_node->right = NULL;
    new_node->type = root->type;
    new_node->data = root->data;
    if (root->type == ND_VAR && root->data.var)
    {
        new_node->data.var = malloc(strlen(root->data.var) + 1);
        if (!new_node->data.var)
        {
            free(new_node);
            return NULL;
        }
        strcpy(new_node->data.var, root->data.var);
    }
    // Если есть левый ребенок, то делаем копирование для него, если он после копирвоания становился NULL,
    // Значит произошла ошибка, и нужно выполнять очистку дерева
    if (root->left)
    {
        new_node->left = copySubTree(root->left);
        if (!new_node->left)
        {
            freeTree(new_node);
            return NULL;
        }
    }
    if (root->right)
    {
        new_node->right = copySubTree(root->right);
        if (!new_node->right)
        {
            freeTree(new_node);
            return NULL;
        }
    }
    return new_node;
}

// Добавить узел на вершину стека
bool pushNode(NodeStack **top, Node *node_ptr)
{
    NodeStack *new_node = malloc(sizeof(NodeStack));
    if (!new_node)
    {
        return 0;
    }
    new_node->tree_node = node_ptr;
    new_node->next = *top;
    *top = new_node;
    return 1;
}

// Достать узел с вершины стека
Node *popNode(NodeStack **top)
{
    if (!(*top))
    {
        return NULL;
    }
    NodeStack *new_node = *top;
    Node *res_tree_node = new_node->tree_node;
    *top = new_node->next;
    free(new_node);
    return res_tree_node;
}

// Очистить стек
void freeStackTree(NodeStack *top)
{
    if (!top)
    {
        return;
    }
    while (top) {
        popNode(&top);
    }
}

// Построить дерево на основе односвязного списка обратной польской записи
Node *buildTree(TokenNode *postfix_head)
{
    if (!postfix_head)
    {
        return NULL;
    }
    NodeStack *tree_stack = NULL;
    TokenNode *curr_token = postfix_head;

    // Идем по токенам в связном списке обратной польской записи
    while (curr_token)
    {
        Node *new_node = createTreeNode(curr_token);
        if (!new_node) {
            freeStackTree(tree_stack);
            return NULL;
        }
        // Если число или переменная - кладем на вершину стека
        if (curr_token->type == TOK_NUM || curr_token->type == TOK_VAR)
        {
            if(!pushNode(&tree_stack, new_node)) {
                freeStackTree(tree_stack);
                return NULL;
            } 
        }
        // Если унарный оператор - берем с вершины стека одно число или меременную,  привязываем её к левому узулу унарного оператора.
        // Сам оператор помещаем на стек
        else if (curr_token->type == TOK_UNARY_MINUS)
        {
            new_node->left = popNode(&tree_stack);
            if(!pushNode(&tree_stack, new_node)) {
                freeStackTree(tree_stack);
                return NULL;
            }
        }
        // Если бинарный оператор - берем с вершины стека два числа или переменных, привязываем их к левому и правому узлам оператора.
        // Сам оператор помещаем на стек
        else
        {
            new_node->right = popNode(&tree_stack);
            new_node->left = popNode(&tree_stack);
            if(!pushNode(&tree_stack, new_node)) {
                freeStackTree(tree_stack);
                return NULL;
            }
        }
        curr_token = curr_token->next;
    }
    // В конце в стеке будет лежать корень дерева
    return popNode(&tree_stack);
}

// Распечатать дерево
void printTreeStructure(Node *root, int level)
{
    if (!root)
    {
        return;
    }
    printTreeStructure(root->right, level + 1);
    for (int i = 0; i < level; i++)
    {
        printf("    ");
    }
    if (root->type == ND_NUM)
    {
        printf("%.2f\n", root->data.num);
    }
    else if (root->type == ND_VAR)
    {
        printf("%s\n", root->data.var);
    }
    else
    {
        printf("%c\n", root->data.op);
    }
    printTreeStructure(root->left, level + 1);
}

// ///////////////////////////////
//      ТРАНСФОРМАЦИЯ ДЕРЕВА
// ///////////////////////////////

// Очистить поддерево и поднять ошибку наверх
int raiseErrorAndFree(Node **root_pointer)
{
    if (*root_pointer)
    {
        freeTree(*root_pointer);
        *root_pointer = NULL;
    }
    return 0;
}

// Провести трансформацию этого дерева. Записывает результат в перемнную по указателю
int makeTransform(Node **root_pointer)
{
    if (!root_pointer || !*root_pointer)
    {
        return 1;
    }
    Node *root = *root_pointer;
    // Если не удалось рекурсивно вызвать трансформацию для детей, то очищаем поддерево с корнем поддерева - текущий узел и поднимаем ошибку наверх
    if (!makeTransform(&(root->left)) || !makeTransform(&(root->right)))
    {
        return raiseErrorAndFree(root_pointer);
    }

    if (root->type == ND_OP && (root->data.op == '+' || root->data.op == '-'))
    {
        Node *l_child = root->left;
        Node *r_child = root->right;
        if (l_child && l_child->type == ND_OP && l_child->data.op == '/' &&
            r_child && r_child->type == ND_OP && r_child->data.op == '/')
        {

            // Переиспользуем уже имеющиеся узлы
            Node *a = l_child->left;
            Node *b = l_child->right;
            Node *c = r_child->left;
            Node *d = r_child->right;

            Node *copy_b = copySubTree(b);
            Node *copy_d = copySubTree(d);

            Node *new_numerator = createOpNode(root->data.op);
            Node *new_denominator = createOpNode('*');

            Node *ab = createOpNode('*');
            Node *bc = createOpNode('*');

            if (!copy_b || !copy_d || !new_numerator || !new_denominator || !ab || !bc)
            {
                freeTree(copy_b);
                freeTree(copy_d);
                freeTree(new_numerator);
                freeTree(new_denominator);
                freeTree(ab);
                freeTree(bc);
                return raiseErrorAndFree(root_pointer);
            }

            ab->left = a;
            ab->right = copy_d;
            bc->left = copy_b;
            bc->right = c;
            new_numerator->left = ab;
            new_numerator->right = bc;
            new_denominator->left = b;
            new_denominator->right = d;

            free(l_child);
            free(r_child);

            root->data.op = '/';
            root->left = new_numerator;
            root->right = new_denominator;
        }
    }
    return 1;
}

// //////////////////////////////////
//      ФОРМАТИРОВАННЫЙ ВЫВОД
// //////////////////////////////////

int getNodePriority(Node *node)
{
    if (!node)
    {
        return -1;
    }
    switch (node->type)
    {
    case ND_NUM:
    case ND_VAR:
        return 3;
    case ND_UNARY:
        return 2;
    case ND_OP:
        switch (node->data.op)
        {
        case '*':
        case '/':
            return 1;
        case '+':
        case '-':
            return 0;
        default:
            return -1;
        }
    default:
        return -1;
    }
}

// Првоерить, нужны ли скобки при печати этого узла
bool needBracket(Node *node, Node *parent, bool is_right_child)
{
    if (!parent)
    {
        return false;
    }
    int priority_curr = getNodePriority(node);
    int priority_parent = getNodePriority(parent);

    // Если эта операция слабее внешней, то скобки нужны
    // (a +[node] b) *[parent] c
    if (priority_curr < priority_parent)
    {
        return true;
    }
    // a -[parent] (b +[node] c) или a /[parent] (b +[node] c)
    if (priority_curr == priority_parent && is_right_child)
    {
        if (parent->type == ND_OP && (parent->data.op == '-' || parent->data.op == '/'))
        {
            return true;
        }
    }
    // Если унарный оператор стоит справа от бинарного, то нужны скобки
    if (node->type == ND_UNARY && parent->type == ND_OP && is_right_child)
    {
        return true;
    }
    return false;
}

// Распечатать дерево выражений в инфиксной записи
void printInfix(Node *node, Node *parent, bool is_right_child)
{
    if (!node)
    {
        return;
    }
    bool need_bracket = needBracket(node, parent, is_right_child);
    if (need_bracket)
    {
        printf("(");
    }
    switch (node->type)
    {
    case ND_NUM:
        printf("%g", node->data.num);
        break;
    case ND_VAR:
        printf("%s", node->data.var);
        break;
    case ND_UNARY:
        printf("-");
        printInfix(node->left, node, false);
        break;
    case ND_OP:
        printInfix(node->left, node, false);
        printf(" %c ", node->data.op);
        printInfix(node->right, node, true);
        break;
    }
    if (need_bracket)
    {
        printf(")");
    }
}

// TODO: ввод целой строки с консоли
int main()
{
    char input[] = "a / (b + c) - (-x) / y";
    printf("Input: %s\n", input);

    TokenNode *tokens = tokenizer(input);
    if (!tokens) {
        fprintf(stderr, "Tokenization error: out of memory.\n");
        exit(1);
    }

    // printTokenList(tokens);

    TokenNode *postfix_tokens = toPostfix(tokens);
    if (!postfix_tokens) {
        freeTokenList(tokens);
        fprintf(stderr, "Deiksta error: out of memory.\n");
        exit(1);
    }

    Node *tree_root = buildTree(postfix_tokens);
    if (!tree_root) {
        freeTokenList(tokens);
        freeTokenList(postfix_tokens);
        fprintf(stderr, "Make tree error: out of memory.\n");
        exit(1);
    }

    printf("\nTree src structure:\n");
    printTreeStructure(tree_root, 0);

    if (!makeTransform(&tree_root))
    {   
        freeTokenList(tokens);
        freeTokenList(postfix_tokens);
        freeTree(tree_root);
        fprintf(stderr, "Transformation error: out of memory.\n");
        exit(1);
    }

    printf("\nTree transformed structure:\n");
    printTreeStructure(tree_root, 0);

    printf("\nOutput:\n");
    printInfix(tree_root, NULL, false);
    printf("\n");

    freeTree(tree_root);
    freeTokenList(tokens);
    freeTokenList(postfix_tokens);

    return 0;
}