#include <stdlib.h>

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

int consume(int ty)
{
    if (tokens[pos]->ty != ty)
        return 0;
    pos++;
    return 1;
}

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
