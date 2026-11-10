//Diogo Henriques - 2019222685
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

Node *root_node = NULL;

Node *newNode(const char *kind, const char *val) {
    Node *n    = malloc(sizeof(Node));
    n->kind    = strdup(kind);
    n->val     = val ? strdup(val) : NULL;
    n->children = NULL;
    n->nchildren = 0;
    n->cap       = 0;
    n->line      = 0;
    n->col       = 0;
    n->type      = NULL;
    return n;
}

void addChild(Node *parent, Node *child) {
    if (!child) return;
    if (parent->nchildren == parent->cap) {
        parent->cap = parent->cap ? parent->cap * 2 : 4;
        parent->children = realloc(parent->children,
                                   parent->cap * sizeof(Node*));
    }
    parent->children[parent->nchildren++] = child;
}

void printAST(Node *n, int depth) {
    if (!n) return;
    for (int i = 0; i < depth * 2; i++) putchar('.');
    if (n->val && n->type)
        printf("%s(%s) - %s\n", n->kind, n->val, n->type);
    else if (n->val)
        printf("%s(%s)\n", n->kind, n->val);
    else if (n->type)
        printf("%s - %s\n", n->kind, n->type);
    else
        printf("%s\n", n->kind);
    for (int i = 0; i < n->nchildren; i++)
        printAST(n->children[i], depth + 1);
}

void freeNode(Node *n) {
    if (!n) return;
    for (int i = 0; i < n->nchildren; i++) freeNode(n->children[i]);
    free(n->children);
    free(n->kind);
    free(n->val);
    free(n->type);
    free(n);
}
