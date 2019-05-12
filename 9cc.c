#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// トークンの型を表す値
enum
{
    TK_NUM = 256, // 整数トークン
    TK_EQ,        // ==
    TK_NE,        // !=
    TK_LE,        // <=
    TK_GE,        // >=
    TK_EOF,       // 入力の終わりを表すトークン
};

// トークンの型
typedef struct
{
    int ty;      // トークンの型
    int val;     // tyがTK_NUMの場合、その数値
    char *input; // トークン文字列（エラーメッセージ用）
} Token;

Token *new_token(int ty, int val, char *input)
{
    Token *token = malloc(sizeof(Token));
    token->ty = ty;
    token->val = val;
    token->input = input;
    return token;
}

// トークナイズした結果のトークン列はこの配列に保存する
Token **tokens;

enum
{
    ND_NUM = 256, // 整数のノードの型
};

typedef struct Node
{
    int ty;           // 演算子かND_NUM
    struct Node *lhs; // 左辺
    struct Node *rhs; // 右辺
    int val;          // tyがND_NUMの場合のみ使う
} Node;

int pos = 0;

typedef struct
{
    void **data;
    int capacity;
    int len;
} Vector;

Vector *new_vector()
{
    Vector *vec = malloc(sizeof(Vector));
    vec->data = malloc(sizeof(void *) * 16);
    vec->capacity = 16;
    vec->len = 0;
    return vec;
}

void vec_push(Vector *vec, void *elem)
{
    if (vec->capacity == vec->len)
    {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len++] = elem;
}

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p)
{
    Vector *vec = new_vector();
    while (*p)
    {
        // 空白文字をスキップ
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (strncmp(p, "==", 2) == 0)
        {
            vec_push(vec, new_token(TK_EQ, 0, p));
            p += 2;
            continue;
        }
        if (strncmp(p, "!=", 2) == 0)
        {
            vec_push(vec, new_token(TK_NE, 0, p));
            p += 2;
            continue;
        }

        if (strncmp(p, "<=", 2) == 0)
        {
            vec_push(vec, new_token(TK_LE, 0, p));
            p += 2;
            continue;
        }
        if (strncmp(p, ">=", 2) == 0)
        {
            vec_push(vec, new_token(TK_GE, 0, p));
            p += 2;
            continue;
        }
        if (*p == '<' || *p == '>')
        {
            vec_push(vec, new_token(*p, 0, p));
            p++;
            continue;
        }

        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')')
        {
            vec_push(vec, new_token(*p, 0, p));
            p++;
            continue;
        }

        if (isdigit(*p))
        {
            vec_push(vec, new_token(TK_NUM, strtol(p, &p, 10), p));
            continue;
        }

        error("トークナイズできません: %s", p);
        exit(1);
    }

    vec_push(vec, new_token(TK_EOF, 0, p));
    tokens = (Token **)vec->data;
}

Node *new_node(int ty, Node *lhs, Node *rhs)
{
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

int consume(int ty)
{
    if (tokens[pos]->ty != ty)
        return 0;
    pos++;
    return 1;
}

Node *add();
Node *equality();

Node *term()
{
    // 次のトークンが'('なら、"(" add ")"のはず
    if (consume('('))
    {
        Node *node = equality();
        if (!consume(')'))
            error("開きカッコに対応する閉じカッコがありません: %s", tokens[pos]->input);
        return node;
    }

    // そうでなければ数値のはず
    if (tokens[pos]->ty == TK_NUM)
        return new_node_num(tokens[pos++]->val);

    error("数値でも開きカッコでもないトークンです: %s", tokens[pos]->input);
}

Node *unary()
{
    if (consume('+'))
        return term();
    if (consume('-'))
        return new_node('-', new_node_num(0), term());
    return term();
}

Node *mul()
{
    Node *node = unary();

    for (;;)
    {
        if (consume('*'))
            node = new_node('*', node, unary());
        else if (consume('/'))
            node = new_node('/', node, unary());
        else
            return node;
    }
}

Node *add()
{
    Node *node = mul();

    for (;;)
    {
        if (consume('+'))
            node = new_node('+', node, mul());
        else if (consume('-'))
            node = new_node('-', node, mul());
        else
            return node;
    }
}

Node *relational()
{
    Node *node = add();

    for (;;)
    {
        if (consume(TK_LE))
            node = new_node(TK_LE, node, add());
        else if (consume(TK_GE))
            node = new_node(TK_LE, add(), node); // 両辺を入れ替えて <=
        else if (consume('<'))
            node = new_node('<', node, add());
        else if (consume('>'))
            node = new_node('<', add(), node); // 両辺を入れ替えて <
        else
            return node;
    }
}

Node *equality()
{
    Node *node = relational();

    for (;;)
    {
        if (consume(TK_EQ))
            node = new_node(TK_EQ, node, relational());
        else if (consume(TK_NE))
            node = new_node(TK_NE, node, relational());
        else
            return node;
    }
}

void gen(Node *node)
{
    if (node->ty == ND_NUM)
    {
        printf("  push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->ty)
    {
    case '+':
        printf("  add rax, rdi\n");
        break;
    case '-':
        printf("  sub rax, rdi\n");
        break;
    case '*':
        printf("  mul rdi\n");
        break;
    case '/':
        printf("  mov rdx, 0\n");
        printf("  div rdi\n");
        break;
    case TK_EQ:
    case TK_NE:
    case TK_LE:
    case '<':
        printf("  cmp rax, rdi\n");
        if (node->ty == TK_EQ)
            printf("  sete al\n");
        else if (node->ty == TK_NE)
            printf("  setne al\n");
        else if (node->ty == TK_LE)
            printf("  setle al\n");
        else if (node->ty == '<')
            printf("  setl al\n");
        printf("  movzb rax, al\n");
    }

    printf("  push rax\n");
}

void runtest();

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }

    if (strcmp(argv[1], "-test") == 0)
    {
        runtest();
        return 0;
    }

    // トークナイズしてパースする
    tokenize(argv[1]);
    Node *node = equality();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコード生成
    gen(node);

    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}

void expect(int line, int expected, int actual)
{
    if (expected == actual)
        return;
    fprintf(stderr, "%d: %d expected, but got %d\n",
            line, expected, actual);
    exit(1);
}

void runtest()
{
    Vector *vec = new_vector();
    expect(__LINE__, 0, vec->len);

    for (int i = 0; i < 100; i++)
        vec_push(vec, (void *)(intptr_t)i);

    expect(__LINE__, 100, vec->len);
    expect(__LINE__, 0, (long)vec->data[0]);
    expect(__LINE__, 50, (long)vec->data[50]);
    expect(__LINE__, 99, (long)vec->data[99]);

    printf("OK\n");
}
