    // Diogo Henriques - 2019222685
%x COMMENT
%x STRING
%{

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif
#include "y.tab.h"

int col = 1;
int lin = 1;

// Posição do token (início do token atual)
int tok_lin = 1;
int tok_col = 1;

// Buffer para strings
char buf_string[8192];
char *ptr_string;
int linha_ini_string;
int col_ini_string;
int string_tem_erros;  // 1 se a string contém sequências de escape inválidas

// Posição de início de comentário não terminado
int linha_ini_com;
int col_ini_com;

// Flags de output 
int flag_l  = 0;  // tokens + erros lexicais
int flag_e1 = 0;  // apenas erros lexicais
int flag_t  = 0;  // modo só-sintático
int flag_e2 = 0;  // apenas erros sintáticos
int flag_s  = 0;  // análise semântica + tabelas + AST anotada
int flag_e3 = 0;  // apenas erros semânticos

/* modo apenas lex: apenas lex, não alimenta o yacc */
int lex_only = 0;

/* exibe o texto do último token (usado em yyerror) */
char tok_text[8210];

/* Regista a posição da última COMMA para detectar vírgula a mais após String[] */
int last_comma_lin = 0, last_comma_col = 0;

/* ativa a atualização do texto do token antes de cada ação de regra */
#define YY_USER_ACTION \
    strncpy(tok_text, yytext, sizeof(tok_text)-1); tok_text[sizeof(tok_text)-1]='\0'; \
    yylloc.first_line = lin; yylloc.first_column = col;

//função auxiliar para imprimir tokens (apenas em modo -l)
void tok(const char *s) {
    if (flag_l) printf("%s\n", s);
}
//função auxiliar para imprimir tokens formatados
void tokf(const char *fmt, const char *val) {
    if (flag_l) printf(fmt, val), printf("\n");
}
%}

LETTER      [a-zA-Z_$]
DIGIT       [0-9]
INT_PART    [0-9]([0-9_]*[0-9])?
FRAC_PART   [0-9]([0-9_]*[0-9])?
EXP_PART    [eE][+-]?[0-9]([0-9_]*[0-9])?

%%

"//"[^\n\r]*      { col += yyleng; /* comentário de uma linha */ }

"/*"                { linha_ini_com = lin; col_ini_com = col; col += yyleng; BEGIN(COMMENT); }
<COMMENT>"*/"       { col += yyleng; BEGIN(INITIAL); }
<COMMENT>\r\n       { lin++; col = 1; }
<COMMENT>\n         { lin++; col = 1; }
<COMMENT>\r         { lin++; col = 1; }
<COMMENT>.          { col += yyleng; }
<COMMENT><<EOF>>    {
                        printf("Line %d, col %d: unterminated comment\n", linha_ini_com, col_ini_com);
                        tok_lin = lin; tok_col = col; tok_text[0] = '\0';
                        BEGIN(INITIAL);
                        yyterminate();
                    }

\"                  {
                        ptr_string        = buf_string;
                        linha_ini_string  = lin;
                        col_ini_string    = col;
                        string_tem_erros  = 0;
                        col++;
                        BEGIN(STRING);
                    }
<STRING>\"          {
                        col++;
                        *ptr_string = '\0';
                        BEGIN(INITIAL);  /* volta ao estado inicial para reconhecer o próximo token */
                        if (!string_tem_erros) {
                            tok_lin = linha_ini_string;
                            tok_col = col_ini_string;
                            snprintf(tok_text, sizeof(tok_text), "\"%.8190s\"", buf_string);
                            if (flag_l)
                                printf("STRLIT(\"%s\")\n", buf_string);
                            if (!lex_only) {
                                yylval.sval = strdup(buf_string);
                                return STRLIT;
                            }
                        }
                    }
<STRING>\r\n        {
                        printf("Line %d, col %d: unterminated string literal\n",
                               linha_ini_string, col_ini_string);
                        lin++; col = 1;
                        BEGIN(INITIAL);
                    }
<STRING>\n          {
                        printf("Line %d, col %d: unterminated string literal\n",
                               linha_ini_string, col_ini_string);
                        lin++; col = 1;
                        BEGIN(INITIAL);
                    }
<STRING>\r          {
                        printf("Line %d, col %d: unterminated string literal\n",
                               linha_ini_string, col_ini_string);
                        lin++; col = 1;
                        BEGIN(INITIAL);
                    }
<STRING><<EOF>>     {
                        printf("Line %d, col %d: unterminated string literal\n",
                               linha_ini_string, col_ini_string);
                        tok_lin = lin; tok_col = col; tok_text[0] = '\0';
                        BEGIN(INITIAL);
                        yyterminate();
                    }
<STRING>\\n         { *ptr_string++ = '\\'; *ptr_string++ = 'n';  col += 2; }
<STRING>\\f         { *ptr_string++ = '\\'; *ptr_string++ = 'f';  col += 2; }
<STRING>\\r         { *ptr_string++ = '\\'; *ptr_string++ = 'r';  col += 2; }
<STRING>\\t         { *ptr_string++ = '\\'; *ptr_string++ = 't';  col += 2; }
<STRING>\\\\        { *ptr_string++ = '\\'; *ptr_string++ = '\\'; col += 2; }
<STRING>\\\"        { *ptr_string++ = '\\'; *ptr_string++ = '"';  col += 2; }
<STRING>\\[^\n\r]  {
                        printf("Line %d, col %d: invalid escape sequence (\\%c)\n",
                               lin, col, yytext[1]);
                        string_tem_erros = 1;
                        col += 2;
                    }
<STRING>\\           {
                        /* barra invertida imediatamente antes de \n, \r, ou EOF */
                        printf("Line %d, col %d: invalid escape sequence (\\)\n",
                               lin, col);
                        string_tem_erros = 1;
                        col++;
                    }
<STRING>[^\\\"\n\r]+ {
                        char *p = yytext;
                        while (*p) *ptr_string++ = *p++;
                        col += yyleng;
                    }

\r\n                { lin++; col = 1; tok_lin = lin; tok_col = col; tok_text[0] = '\0'; }
\n                  { lin++; col = 1; tok_lin = lin; tok_col = col; tok_text[0] = '\0'; }
\r                  { lin++; col = 1; tok_lin = lin; tok_col = col; tok_text[0] = '\0'; }
[ \t\f\v]+          { col += yyleng; tok_lin = lin; tok_col = col; tok_text[0] = '\0'; }

"System.out.print"   { tok("PRINT");      tok_lin=lin; tok_col=col; strncpy(tok_text,yytext,sizeof(tok_text)-1); col+=yyleng; if(!lex_only) return PRINT;     }
"Integer.parseInt"   { tok("PARSEINT");   tok_lin=lin; tok_col=col; strncpy(tok_text,yytext,sizeof(tok_text)-1); col+=yyleng; if(!lex_only) return PARSEINT;  }

"boolean"   { tok("BOOL");      tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return BOOL;      }
"class"     { tok("CLASS");     tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return CLASS;     }
".length"   { tok("DOTLENGTH"); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return DOTLENGTH; }
"double"    { tok("DOUBLE");    tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return DOUBLE;    }
"else"      { tok("ELSE");      tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return ELSE;      }
"if"        { tok("IF");        tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return IF;        }
"int"       { tok("INT");       tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return INT;       }
"public"    { tok("PUBLIC");    tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return PUBLIC;    }
"return"    { tok("RETURN");    tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return RETURN;    }
"static"    { tok("STATIC");    tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return STATIC;    }
"String"    { tok("STRING");    tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return STRING_KW; }
"void"      { tok("VOID");      tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return VOID;      }
"while"     { tok("WHILE");     tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return WHILE;     }

"true"      {
                tok("BOOLLIT(true)");
                tok_lin=lin; tok_col=col; col+=yyleng;
                if(!lex_only) { yylval.sval=strdup(yytext); return BOOLLIT; }
            }
"false"     {
                tok("BOOLLIT(false)");
                tok_lin=lin; tok_col=col; col+=yyleng;
                if(!lex_only) { yylval.sval=strdup(yytext); return BOOLLIT; }
            }

"abstract"     { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"assert"       { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"break"        { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"byte"         { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"case"         { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"catch"        { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"char"         { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"const"        { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"continue"     { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"default"      { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"do"           { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"enum"         { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"extends"      { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"final"        { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"finally"      { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"float"        { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"for"          { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"goto"         { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"implements"   { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"import"       { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"instanceof"   { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"interface"    { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"long"         { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"native"       { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"new"          { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"null"         { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"package"      { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"private"      { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"protected"    { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"short"        { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"strictfp"     { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"super"        { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"switch"       { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"synchronized" { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"this"         { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"throw"        { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"throws"       { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"transient"    { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"try"          { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"volatile"     { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"Integer"      { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"System"       { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"++"           { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }
"--"           { tokf("RESERVED(%s)", yytext); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) { yylval.sval=strdup(yytext); return RESERVED; } }

"&&"   { tok("AND");       tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return AND;       }
"||"   { tok("OR");        tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return OR;        }
"=="   { tok("EQ");        tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return EQ;        }
"!="   { tok("NE");        tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return NE;        }
">="   { tok("GE");        tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return GE;        }
"<="   { tok("LE");        tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return LE;        }
"<<"   { tok("LSHIFT");    tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return LSHIFT;    }
">>"   { tok("RSHIFT");    tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return RSHIFT;    }
"->"   { tok("ARROW");     tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return ARROW;     }
"="    { tok("ASSIGN");    tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return ASSIGN;    }
">"    { tok("GT");        tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return GT;        }
"<"    { tok("LT");        tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return LT;        }
"+"    { tok("PLUS");      tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return PLUS;      }
"-"    { tok("MINUS");     tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return MINUS;     }
"*"    { tok("STAR");      tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return STAR;      }
"/"    { tok("DIV");       tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return DIV;       }
"%"    { tok("MOD");       tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return MOD;       }
"!"    { tok("NOT");       tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return NOT;       }
"^"    { tok("XOR");       tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return XOR;       }
"{"    { tok("LBRACE");    tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return LBRACE;    }
"}"    { tok("RBRACE");    tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return RBRACE;    }
"("    { tok("LPAR");      tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return LPAR;      }
")"    { tok("RPAR");      tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return RPAR;      }
"["    { tok("LSQ");       tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return LSQ;       }
"]"    { tok("RSQ");       tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return RSQ;       }
";"    { tok("SEMICOLON"); tok_lin=lin; tok_col=col; col+=yyleng; if(!lex_only) return SEMICOLON; }
","    { tok("COMMA");     tok_lin=lin; tok_col=col; last_comma_lin=lin; last_comma_col=col; col+=yyleng; if(!lex_only) return COMMA;     }

{INT_PART}"."{FRAC_PART}?{EXP_PART}?  {
                        if (flag_l) printf("DECIMAL(%s)\n", yytext);
                        tok_lin=lin; tok_col=col; col += yyleng;
                        if(!lex_only) { yylval.sval=strdup(yytext); return DECIMAL; }
                    }
"."{FRAC_PART}{EXP_PART}?              {
                        if (flag_l) printf("DECIMAL(%s)\n", yytext);
                        tok_lin=lin; tok_col=col; col += yyleng;
                        if(!lex_only) { yylval.sval=strdup(yytext); return DECIMAL; }
                    }
{INT_PART}{EXP_PART}                   {
                        if (flag_l) printf("DECIMAL(%s)\n", yytext);
                        tok_lin=lin; tok_col=col; col += yyleng;
                        if(!lex_only) { yylval.sval=strdup(yytext); return DECIMAL; }
                    }

"0"                         {
                        tok("NATURAL(0)");
                        tok_lin=lin; tok_col=col; col += yyleng;
                        if(!lex_only) { yylval.sval=strdup(yytext); return NATURAL; }
                    }
[1-9]([0-9_]*[0-9])?        {
                        if (flag_l) printf("NATURAL(%s)\n", yytext);
                        tok_lin=lin; tok_col=col; col += yyleng;
                        if(!lex_only) { yylval.sval=strdup(yytext); return NATURAL; }
                    }

{LETTER}({LETTER}|{DIGIT})*  {
                        if (flag_l) printf("IDENTIFIER(%s)\n", yytext);
                        tok_lin=lin; tok_col=col; col += yyleng;
                        if(!lex_only) { yylval.sval=strdup(yytext); return IDENTIFIER; }
                    }

.                   {
                        printf("Line %d, col %d: illegal character (%s)\n",
                               lin, col, yytext);
                        col += yyleng;
                    }

%%

int main(int argc, char *argv[]) {
#ifdef _WIN32
    _setmode(_fileno(stdout), _O_BINARY);
    _setmode(_fileno(stdin),  _O_BINARY);
#endif
    flag_l  = 0;
    flag_e1 = 0;
    flag_t  = 0;
    flag_e2 = 0;
    flag_s  = 0;
    flag_e3 = 0;

    for (int i = 1; i < argc; i++) {
        if      (strcmp(argv[i], "-l")  == 0) flag_l  = 1;
        else if (strcmp(argv[i], "-e1") == 0) flag_e1 = 1;
        else if (strcmp(argv[i], "-t")  == 0) flag_t  = 1;
        else if (strcmp(argv[i], "-e2") == 0) flag_e2 = 1;
        else if (strcmp(argv[i], "-s")  == 0) flag_s  = 1;
        else if (strcmp(argv[i], "-e3") == 0) flag_e3 = 1;
    }

    if (flag_l || flag_e1) {
        // Meta 1 :  lex 
        lex_only = 1;
        if (!flag_l && !flag_e1) flag_e1 = 1;
        yylex();
        return 0;
    }

    // Meta 2/3: full parse 
    lex_only = 0;
    yyparse();
    { extern void do_print_after_parse(void); do_print_after_parse(); }
    return 0;
}

int yywrap(void) {
    return 1;
}
