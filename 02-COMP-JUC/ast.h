//Diogo Henriques - 2019222685
#ifndef AST_H
#define AST_H

/* ------------------------------------------------------------------ */
/*  Nó da AST                                                          */
/* ------------------------------------------------------------------ */
typedef struct Node {
    char        *kind;   /* nome tipo de nó, ex: "Program", "Add", … */
    char        *val;    /* valor opcional(identifiers, literals)    */
    struct Node **children;
    int          nchildren;
    int          cap;
    int          line;   /* linha do token principal deste nó        */
    int          col;    /* coluna do token principal deste nó       */
    char        *type;   /* tipo semântico anotado (ex: "int")       */
} Node;

extern Node *root_node;  /* raiz da AST guardada para impressão dps de parse */

Node *newNode(const char *kind, const char *val);
void  addChild(Node *parent, Node *child);
void  printAST(Node *n, int depth);
void  freeNode(Node *n);

#endif /* AST_H */
