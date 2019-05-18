// トークンの型を表す値
enum
{
    TK_NUM = 256, // 整数トークン
    TK_IDENT,     // 識別子
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

enum
{
    ND_NUM = 256, // 整数のノードの型
    ND_IDENT,     // 識別子
};

typedef struct Node
{
    int ty;           // 演算子かND_NUM
    struct Node *lhs; // 左辺
    struct Node *rhs; // 右辺
    int val;          // tyがND_NUMの場合のみ使う
    char name;        // tyがND_IDENTの場合のみ使う
} Node;

typedef struct
{
    void **data;
    int capacity;
    int len;
} Vector;

extern Token **tokens;
extern int pos;
extern Node *code[100];

void error(char *fmt, ...);

void tokenize(char *p);

void gen(Node *node);
void program();

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
