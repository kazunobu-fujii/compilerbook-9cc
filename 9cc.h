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

typedef struct
{
    void **data;
    int capacity;
    int len;
} Vector;

extern Token **tokens;
extern int pos;

void error(char *fmt, ...);

void tokenize(char *p);

void gen(Node *node);
Node *equality();

Vector *new_vector();
void vec_push(Vector *vec, void *elem);
