//Diogo Henriques - 2019222685
%locations
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"
#include "semantics.h"
#include "codegen.h"

// Variáveis globais do lexer
extern int  lin, col;
extern int  tok_lin, tok_col;
extern int  flag_t, flag_e2, flag_s, flag_e3;
extern int  lex_only;
extern char tok_text[];  // texto do token 

int  syntax_errors = 0;


int  g_use_comma_error = 0;
int  g_comma_error_line = 0, g_comma_error_col = 0;
int  g_in_string_param = 0;  /* 1 se o último FormalParamList foi um parâmetro String[] */
extern int last_comma_lin, last_comma_col;  /* do lexer */

extern int  yylex(void);

void yyerror(const char *s) {
    if (g_in_string_param && tok_text[0] == ')') {
        /* Vírgula a mais após parâmetro String[]: reportar na posição da vírgula */
        printf("Line %d, col %d: %s: ,\n", last_comma_lin, last_comma_col, s);
        g_in_string_param = 0;
    } else {
        printf("Line %d, col %d: %s: %s\n", tok_lin, tok_col, s, tok_text);
    }
    syntax_errors++;
}
%}


%union {
    char *sval;
    struct Node *node;
}

//Definição dos tokens
%token <sval> IDENTIFIER NATURAL DECIMAL BOOLLIT STRLIT RESERVED
%token AND OR EQ NE GE GT LE LT LSHIFT RSHIFT
%token ASSIGN PLUS MINUS STAR DIV MOD NOT XOR ARROW
%token LBRACE RBRACE LPAR RPAR LSQ RSQ SEMICOLON COMMA DOTLENGTH
%token BOOL CLASS DOUBLE ELSE IF INT PUBLIC RETURN STATIC STRING_KW VOID WHILE
%token PRINT PARSEINT

//Precedência e associatividade 
%nonassoc ASSIGN
%left  OR
%left  AND
%left  XOR
%left  EQ NE
%left  LT GT LE GE
%left  LSHIFT RSHIFT
%left  PLUS MINUS
%left  STAR DIV MOD
%right UMINUS UPLUS UNOT   

/* Prefere shift (associa ELSE com o IF mais próximo) */
%nonassoc LOWER_ELSE
%nonassoc ELSE

//tipo dos não-terminais
%type <node> Program
%type <node> ClassMemberList ClassMember
%type <node> MethodDecl FieldDecl
%type <node> Type
%type <node> MethodHeader FormalParams FormalParamList FormalParam
%type <node> MethodBody StmtOrVarList StmtOrVar
%type <node> VarDecl VarIdList
%type <node> Statement
%type <node> StmtList
%type <node> MethodInvocation AssignableExpr ParseArgs
%type <node> ExprList ExprListNE
%type <node> Expr
%type <node> OptExpr

%%


Start:
    Program
        {
            root_node = $1;  //define raiz da AST para print depois do parse
        }
;


Program:
    CLASS IDENTIFIER LBRACE ClassMemberList RBRACE
        {
            $$ = newNode("Program", NULL);
            addChild($$, newNode("Identifier", $2)); free($2);
            Node *list = $4;
            for (int i = 0; i < list->nchildren; i++)
                addChild($$, list->children[i]);
            //limpa memória do wrapper list (children já foram roubados acima)
            free(list->children); free(list->kind); free(list->val); free(list);
        }
;

ClassMemberList:
    /* empty */
        { $$ = newNode("__list__", NULL); }
  | ClassMemberList ClassMember
        { $$ = $1;
          if ($2) {
              if (strcmp($2->kind, "__fieldlist__") == 0) {
                  for (int i = 0; i < $2->nchildren; i++) addChild($$, $2->children[i]);
                  free($2->children); free($2->kind); free($2->val); free($2);
              } else {
                  addChild($$, $2);
              }
          }
        }
  | ClassMemberList SEMICOLON
        { $$ = $1; }
;

ClassMember:
    MethodDecl  { $$ = $1; }
  | FieldDecl   { $$ = $1; }
;

/* ================================================================== */
MethodDecl:
    PUBLIC STATIC MethodHeader MethodBody
        {
            $$ = newNode("MethodDecl", NULL);
            addChild($$, $3);
            addChild($$, $4);
        }
;

/* ================================================================== */
FieldDecl:
    PUBLIC STATIC Type IDENTIFIER VarIdList SEMICOLON
        {
            /* Um FieldDecl por cada identificador declarado */
            /* Devolve um nó temporário; o pai voltará a adicionar os filhos */
            $$ = newNode("__fieldlist__", NULL);
            Node *fd = newNode("FieldDecl", NULL);
            addChild(fd, $3);
            Node *id4 = newNode("Identifier", $4); free($4);
            id4->line = @4.first_line; id4->col = @4.first_column;
            addChild(fd, id4);
            addChild($$, fd);
            /* identificadores adicionais */
            for (int i = 0; i < $5->nchildren; i++) {
                Node *fdi = newNode("FieldDecl", NULL);
                Node *typeCopy = newNode($3->kind, $3->val);
                addChild(fdi, typeCopy);
                addChild(fdi, $5->children[i]);
                addChild($$, fdi);
            }
            /* liberta o wrapper da lista (filhos já transferidos acima) */
            free($5->children); free($5->kind); free($5->val); free($5);
        }
  | error SEMICOLON
        { $$ = NULL; }
;

/* Lista de identificadores extra após o primeiro: ", id, id, ..." */
VarIdList:
    /* empty */
        { $$ = newNode("__idlist__", NULL); }
  | VarIdList COMMA IDENTIFIER
        {
            $$ = $1;
            Node *id3 = newNode("Identifier", $3); free($3);
            id3->line = @3.first_line; id3->col = @3.first_column;
            addChild($$, id3);
        }
;

/* ================================================================== */
Type:
    BOOL    { $$ = newNode("Bool",   NULL); }
  | INT     { $$ = newNode("Int",    NULL); }
  | DOUBLE  { $$ = newNode("Double", NULL); }
;

/* ================================================================== */
MethodHeader:
    Type IDENTIFIER LPAR FormalParams RPAR
        {
            $$ = newNode("MethodHeader", NULL);
            addChild($$, $1);
            Node *id2 = newNode("Identifier", $2); free($2);
            id2->line = @2.first_line; id2->col = @2.first_column;
            addChild($$, id2);
            addChild($$, $4);
        }
  | VOID IDENTIFIER LPAR FormalParams RPAR
        {
            $$ = newNode("MethodHeader", NULL);
            addChild($$, newNode("Void", NULL));
            Node *id2 = newNode("Identifier", $2); free($2);
            id2->line = @2.first_line; id2->col = @2.first_column;
            addChild($$, id2);
            addChild($$, $4);
        }
;

FormalParams:
    /* empty */          { g_in_string_param = 0; $$ = newNode("MethodParams", NULL); }
  | FormalParamList      { g_in_string_param = 0; $$ = $1; }
;

FormalParamList:
    FormalParam
        {
            g_in_string_param = 0;
            $$ = newNode("MethodParams", NULL);
            addChild($$, $1);
        }
  | FormalParamList COMMA FormalParam
        {
            g_use_comma_error = 0;
            g_in_string_param = 0;
            $$ = $1;
            addChild($$, $3);
        }
  | STRING_KW LSQ RSQ IDENTIFIER
        {
            g_in_string_param = 1;  /* Marcar: último parâmetro foi String[] */
            $$ = newNode("MethodParams", NULL);
            Node *pd = newNode("ParamDecl", NULL);
            addChild(pd, newNode("StringArray", NULL));
            Node *id4 = newNode("Identifier", $4); free($4);
            id4->line = @4.first_line; id4->col = @4.first_column;
            addChild(pd, id4);
            addChild($$, pd);
        }
;

FormalParam:
    Type IDENTIFIER
        {
            $$ = newNode("ParamDecl", NULL);
            addChild($$, $1);
            Node *id2 = newNode("Identifier", $2); free($2);
            id2->line = @2.first_line; id2->col = @2.first_column;
            addChild($$, id2);
        }
;

/* ================================================================== */
MethodBody:
    LBRACE StmtOrVarList RBRACE
        {
            $$ = newNode("MethodBody", NULL);
            Node *list = $2;
            for (int i = 0; i < list->nchildren; i++)
                addChild($$, list->children[i]);
            free(list->children); free(list->kind); free(list->val); free(list);
        }
;

StmtOrVarList:
    /* empty */            { $$ = newNode("__list__", NULL); }
  | StmtOrVarList StmtOrVar
        { $$ = $1; if ($2) { /* VarDecl expande para vários filhos no estilo FieldDecl */
            if (strcmp($2->kind, "__fieldlist__") == 0) {
                for (int i = 0; i < $2->nchildren; i++) addChild($$, $2->children[i]);
                free($2->children); free($2->kind); free($2->val); free($2);
            } else {
                addChild($$, $2);
            }
          }
        }
;

StmtOrVar:
    Statement  { $$ = $1; }
  | VarDecl    { $$ = $1; }
;


VarDecl:
    Type IDENTIFIER VarIdList SEMICOLON
        {
            $$ = newNode("__fieldlist__", NULL);
            Node *vd = newNode("VarDecl", NULL);
            addChild(vd, $1);
            Node *id2 = newNode("Identifier", $2); free($2);
            id2->line = @2.first_line; id2->col = @2.first_column;
            addChild(vd, id2);
            addChild($$, vd);
            for (int i = 0; i < $3->nchildren; i++) {
                Node *vdi = newNode("VarDecl", NULL);
                Node *typeCopy = newNode($1->kind, $1->val);
                addChild(vdi, typeCopy);
                addChild(vdi, $3->children[i]);
                addChild($$, vdi);
            }
            free($3->children); free($3->kind); free($3->val); free($3);
        }
;

Statement:
    LBRACE StmtList RBRACE
        {
            // Bloco(!=1): 0 filhos→NULL, 1 filho→promove, 2+→Block 
            Node *list = $2;
            if (list->nchildren == 0) {
                $$ = NULL;  // bloco vazio: Block obrigatório só para If/While 
                free(list->children); free(list->kind); free(list->val); free(list);
            } else if (list->nchildren == 1) {
                $$ = list->children[0];
                free(list->children); free(list->kind); free(list->val); free(list);
            } else {
                $$ = newNode("Block", NULL);
                for (int i = 0; i < list->nchildren; i++) addChild($$, list->children[i]);
                free(list->children); free(list->kind); free(list->val); free(list);
            }
        }
  | IF LPAR AssignableExpr RPAR Statement %prec LOWER_ELSE
        {
            $$ = newNode("If", NULL);
            addChild($$, $3);
            addChild($$, $5 ? $5 : newNode("Block", NULL)); /* ramo then obrigatório */
            addChild($$, newNode("Block", NULL));            /* ramo else vazio obrigatório */
        }
  | IF LPAR AssignableExpr RPAR Statement ELSE Statement
        {
            $$ = newNode("If", NULL);
            addChild($$, $3);
            addChild($$, $5 ? $5 : newNode("Block", NULL)); /* ramo then obrigatório */
            addChild($$, $7 ? $7 : newNode("Block", NULL)); /* ramo else obrigatório */
        }
  | WHILE LPAR AssignableExpr RPAR Statement
        {
            $$ = newNode("While", NULL);
            addChild($$, $3);
            addChild($$, $5 ? $5 : newNode("Block", NULL)); /* body obrigatório */
        }
  | RETURN OptExpr SEMICOLON
        {
            $$ = newNode("Return", NULL);
            $$->line = @1.first_line; $$->col = @1.first_column;
            if ($2) addChild($$, $2);
        }
  | SEMICOLON
        { $$ = NULL; }
  | MethodInvocation SEMICOLON
        { $$ = $1; }
  | IDENTIFIER ASSIGN AssignableExpr SEMICOLON
        {
            $$ = newNode("Assign", NULL);
            $$->line = @2.first_line; $$->col = @2.first_column;
            Node *id1 = newNode("Identifier", $1); free($1);
            id1->line = @1.first_line; id1->col = @1.first_column;
            addChild($$, id1);
            addChild($$, $3);
        }
  | ParseArgs SEMICOLON
        { $$ = $1; }
  | PRINT LPAR AssignableExpr RPAR SEMICOLON
        {
            $$ = newNode("Print", NULL);
            addChild($$, $3);
        }
  | PRINT LPAR STRLIT RPAR SEMICOLON
        {
            $$ = newNode("Print", NULL);
            { // envolve o valor da string em aspas  na AST
                char *q = malloc(strlen($3) + 3);
                sprintf(q, "\"%s\"", $3);
                addChild($$, newNode("StrLit", q));
                free(q);
            }
            free($3);
        }
  | error SEMICOLON
        { $$ = NULL; }
;

StmtList:
    /* vazio */         { $$ = newNode("__list__", NULL); }
  | StmtList Statement  { $$ = $1; if ($2) addChild($$, $2); }
;

OptExpr:
    /* vazio */ { $$ = NULL; }
  | AssignableExpr { $$ = $1; }
;

/* ================================================================== */
MethodInvocation:
    IDENTIFIER LPAR ExprList RPAR
        {
            $$ = newNode("Call", NULL);
            $$->line = @1.first_line; $$->col = @1.first_column;
            Node *id1 = newNode("Identifier", $1); free($1);
            id1->line = @1.first_line; id1->col = @1.first_column;
            addChild($$, id1);
            Node *list = $3;
            for (int i = 0; i < list->nchildren; i++) addChild($$, list->children[i]);
            free(list->children); free(list->kind); free(list->val); free(list);
        }
  | IDENTIFIER LPAR error RPAR
        {
            $$ = newNode("Call", NULL);
            $$->line = @1.first_line; $$->col = @1.first_column;
            Node *id1 = newNode("Identifier", $1); free($1);
            id1->line = @1.first_line; id1->col = @1.first_column;
            addChild($$, id1);
        }
;

/* AssignableExpr:  Usada em argumentos de chamada e condições de if/while*/
AssignableExpr:
    IDENTIFIER ASSIGN AssignableExpr
        {
            $$ = newNode("Assign", NULL);
            $$->line = @2.first_line; $$->col = @2.first_column;
            Node *id1 = newNode("Identifier", $1); free($1);
            id1->line = @1.first_line; id1->col = @1.first_column;
            addChild($$, id1);
            addChild($$, $3);
        }
  | Expr
        { $$ = $1; }
;

ParseArgs:
    PARSEINT LPAR IDENTIFIER LSQ AssignableExpr RSQ RPAR
        {
            $$ = newNode("ParseArgs", NULL);
            $$->line = @1.first_line; $$->col = @1.first_column;
            Node *id3 = newNode("Identifier", $3); free($3);
            id3->line = @3.first_line; id3->col = @3.first_column;
            addChild($$, id3);
            addChild($$, $5);
        }
  | PARSEINT LPAR error RPAR
        {
            $$ = newNode("ParseArgs", NULL);
            $$->line = @1.first_line; $$->col = @1.first_column;
        }
;


ExprList:
    /* empty */    { $$ = newNode("__list__", NULL); }
  | ExprListNE    { $$ = $1; }
;

ExprListNE:
    AssignableExpr
        {
            $$ = newNode("__list__", NULL);
            addChild($$, $1);
        }
  | ExprListNE COMMA AssignableExpr
        { $$ = $1; addChild($$, $3); }
;


Expr:
    Expr PLUS   Expr  { $$ = newNode("Add",    NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr MINUS  Expr  { $$ = newNode("Sub",    NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr STAR   Expr  { $$ = newNode("Mul",    NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr DIV    Expr  { $$ = newNode("Div",    NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr MOD    Expr  { $$ = newNode("Mod",    NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr AND    Expr  { $$ = newNode("And",    NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr OR     Expr  { $$ = newNode("Or",     NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr XOR    Expr  { $$ = newNode("Xor",    NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr LSHIFT Expr  { $$ = newNode("Lshift", NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr RSHIFT Expr  { $$ = newNode("Rshift", NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr EQ     Expr  { $$ = newNode("Eq",     NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr NE     Expr  { $$ = newNode("Ne",     NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr LT     Expr  { $$ = newNode("Lt",     NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr GT     Expr  { $$ = newNode("Gt",     NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr LE     Expr  { $$ = newNode("Le",     NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | Expr GE     Expr  { $$ = newNode("Ge",     NULL); $$->line=@2.first_line; $$->col=@2.first_column; addChild($$,$1); addChild($$,$3); }
  | MINUS Expr %prec UMINUS  { $$ = newNode("Minus", NULL); $$->line=@1.first_line; $$->col=@1.first_column; addChild($$,$2); }
  | PLUS  Expr %prec UPLUS   { $$ = newNode("Plus",  NULL); $$->line=@1.first_line; $$->col=@1.first_column; addChild($$,$2); }
  | NOT   Expr %prec UNOT    { $$ = newNode("Not",   NULL); $$->line=@1.first_line; $$->col=@1.first_column; addChild($$,$2); }
  | LPAR AssignableExpr RPAR  { $$ = $2; }
  | LPAR error RPAR           { $$ = NULL; }
  | MethodInvocation          { $$ = $1; }
  | ParseArgs                 { $$ = $1; }
  | IDENTIFIER DOTLENGTH
        {
            $$ = newNode("Length", NULL);
            $$->line = @2.first_line; $$->col = @2.first_column;
            Node *id1 = newNode("Identifier", $1); free($1);
            id1->line = @1.first_line; id1->col = @1.first_column;
            addChild($$, id1);
        }
  | IDENTIFIER
        {
            $$ = newNode("Identifier", $1); free($1);
            $$->line = @1.first_line; $$->col = @1.first_column;
        }
  | NATURAL
        {
            $$ = newNode("Natural", $1); free($1);
            $$->line = @1.first_line; $$->col = @1.first_column;
        }
  | DECIMAL
        {
            $$ = newNode("Decimal", $1); free($1);
            $$->line = @1.first_line; $$->col = @1.first_column;
        }
  | BOOLLIT
        {
            $$ = newNode("BoolLit", $1); free($1);
            $$->line = @1.first_line; $$->col = @1.first_column;
        }
;

%%

/* Chamada a partir do main() em lex.yy.c após yyparse(),
   para que todos os erros do lex apareçam na saída ANTES da AST. */
void do_print_after_parse(void) {
    if (flag_t) {
        // -t: análise sintática + AST; -e2: só erros sintáticos
        if (!syntax_errors && flag_s && root_node)
            printAST(root_node, 0);
        freeNode(root_node);
        root_node = NULL;
        return;
    }
    // -s, -e3, -e2, ou sem flags: executar análise semântica
    if (!syntax_errors && root_node) {
        do_semantic(root_node, flag_s);
        // Geração de código: só sem flags especiais E sem erros
        if (!flag_e2 && !flag_s && !flag_e3
                && syntax_errors == 0 && semantic_errors == 0) {
            codegen_program(root_node);
        }
    }
    freeNode(root_node);
    root_node = NULL;
}
