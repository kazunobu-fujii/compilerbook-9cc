#include <stdlib.h>
#include <stdio.h>

#include "9cc.h"

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

Node *new_node_ident(char *name)
{
    Node *node = malloc(sizeof(Node));
    node->ty = ND_IDENT;
    node->name = *name;
    return node;
}

int consume(int ty)
{
    if (tokens[pos]->ty != ty)
        return 0;
    pos++;
    return 1;
}

Node *expr();

Node *term()
{
    // 次のトークンが'('なら、"(" add ")"のはず
    if (consume('('))
    {
        Node *node = expr();
        if (!consume(')'))
            error("開きカッコに対応する閉じカッコがありません: %s", tokens[pos]->input);
        return node;
    }

    // そうでなければ数値のはず
    if (tokens[pos]->ty == TK_NUM)
        return new_node_num(tokens[pos++]->val);

    // または変数
    if (tokens[pos]->ty == TK_IDENT)
        return new_node_ident(tokens[pos++]->input);

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

Node *code[100];

Node *assign()
{
    Node *node = equality();
    if (consume('='))
        node = new_node('=', node, assign());
    return node;
}

Node *expr()
{
    return assign();
}

Node *stmt()
{
    Node *node = expr();
    if (!consume(';'))
        error("';'ではないトークンです: %s", tokens[pos]->input);
    return node;
}

void program()
{
    int i = 0;
    while (tokens[pos]->ty != TK_EOF)
        code[i++] = stmt();
    code[i] = NULL;
}

void gen_lval(Node *node)
{
    if (node->ty != ND_IDENT)
        error("代入の左辺値が変数ではありません");

    int offset = ('z' - node->name + 1) * 8;
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", offset);
    printf("  push rax\n");
}

void gen(Node *node)
{
    if (node->ty == ND_NUM)
    {
        printf("  push %d\n", node->val);
        return;
    }

    if (node->ty == ND_IDENT)
    {
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    }

    if (node->ty == '=')
    {
        gen_lval(node->lhs);
        gen(node->rhs);

        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
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
