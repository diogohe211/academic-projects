// Diogo Henriques - 2019222685
#ifndef SEMANTICS_H
#define SEMANTICS_H

#include "ast.h"

// Strings de nome de tipo

#define T_INT      "int"
#define T_DOUBLE   "double"
#define T_BOOL     "boolean"
#define T_VOID     "void"
#define T_STRARRAY "String[]"
#define T_UNDEF    "undef"


// Símbolo local (entrada na tabela de símbolos de um método)
typedef struct LocalSym {
    char *name;       // nome do identificador (ou "return")
    char *type;       // string de tipo
    int   is_param;   // 1 = parâmetro formal, 0 = local/return
    int   line, col;  // posição da declaração
    struct LocalSym *next;
} LocalSym;

// Símbolo de classe (entrada na tabela de símbolos ao nível da classe)
typedef struct ClassSym {
    char *name;        // nome do identificador
    int   is_method;   // 1 = método, 0 = campo
    char *type;        // tipo do campo OU tipo de retorno do método
    // Apenas métodos:
    char  *param_sig;  // ex: "(int,String[])"
    char **param_types;// array de strings de tipos dos parâmetros
    int    num_params;
    LocalSym *locals;  // tabela local do método (return+params+vars)
    // Posição do identificador no código fonte:
    int   line, col;
    struct ClassSym *next;
} ClassSym;

// Variáveis globais
extern ClassSym *g_class_table;
extern char     *g_class_name;
extern int       semantic_errors;

// Ponto de entrada principal
void do_semantic(Node *root, int print_output);

#endif // SEMANTICS_H
