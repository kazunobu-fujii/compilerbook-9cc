#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "9cc.h"

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

int pos = 0;

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
