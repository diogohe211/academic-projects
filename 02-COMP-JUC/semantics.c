// Diogo Henriques - 2019222685
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include "ast.h"
#include "semantics.h"

//  Variáveis globais                                   
ClassSym *g_class_table = NULL;
char     *g_class_name  = NULL;
int       semantic_errors = 0;

//   acesso  a filho da AST                             
static Node *child(Node *n, int i) {
    if (!n || i < 0 || i >= n->nchildren) return NULL;
    return n->children[i];
}

//  Auxiliares de tipos    
// Nome do nó de tipo → string de tipo 
static const char *kind_to_type(const char *kind) {
    if (!kind) return T_UNDEF;
    if (strcmp(kind, "Int")         == 0) return T_INT;
    if (strcmp(kind, "Double")      == 0) return T_DOUBLE;
    if (strcmp(kind, "Bool")        == 0) return T_BOOL;
    if (strcmp(kind, "Void")        == 0) return T_VOID;
    if (strcmp(kind, "StringArray") == 0) return T_STRARRAY;
    return T_UNDEF;
}

// O tipo 'from' é compatível por alargamento com 'to' para atribuição/retorno? 
static int type_widens(const char *from, const char *to) {
    if (strcmp(from, T_UNDEF) == 0 || strcmp(to, T_UNDEF) == 0) return 0;
    if (strcmp(from, T_STRARRAY) == 0 || strcmp(to, T_STRARRAY) == 0) return 0;
    if (strcmp(from, to) == 0) return 1;
    if (strcmp(from, T_INT) == 0 && strcmp(to, T_DOUBLE) == 0) return 1;
    return 0;
}

// Dois tipos são iguais (e não undef)? 
static int types_equal(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

// O tipo é numérico (int ou double)? 
static int is_numeric(const char *t) {
    return strcmp(t, T_INT) == 0 || strcmp(t, T_DOUBLE) == 0;
}

// Tipo resultante de uma operação aritmética binária (ou undef se tipos inválidos)
static const char *arith_result(const char *a, const char *b) {
    if (!is_numeric(a) || !is_numeric(b)) return T_UNDEF;
    if (strcmp(a, T_DOUBLE) == 0 || strcmp(b, T_DOUBLE) == 0) return T_DOUBLE;
    return T_INT;
}

// Verifica se um literal Natural é > INT_MAX 
static int natural_out_of_bounds(const char *s) {
    /* Remove underscores */
    char clean[256]; int j = 0;
    for (int i = 0; s[i] && j < 255; i++)
        if (s[i] != '_') clean[j++] = s[i];
    clean[j] = '\0';
    int len = (int)strlen(clean);
    if (len > 10) return 1;
    if (len < 10) return 0;
    /* Exactamente 10 dígitos: comparar string */
    return strcmp(clean, "2147483647") > 0;
}

// Verifica se um literal Decimal está fora do intervalo double (overflow ou underflow para 0) 
static int decimal_out_of_bounds(const char *s) {
    /* Remove underscores */
    char clean[512]; int j = 0;
    for (int i = 0; s[i] && j < 511; i++)
        if (s[i] != '_') clean[j++] = s[i];
    clean[j] = '\0';
    errno = 0;
    double v = strtod(clean, NULL);
    /* Underflow (arredonda para 0) ou overflow (infinito) */
    if (errno == ERANGE && (v == 0.0 || v == (double)HUGE_VAL || v == -(double)HUGE_VAL))
        return 1;
    return 0;
}

// Constrói a string de assinatura de parâmetros "(int,double)" a partir de um array
static char *make_param_sig(char **types, int n) {
    // Calcular tamanho necessário 2 para parênteses + strlen de cada tipo + vírgulas
    int sz = 2; // "(" e ")" 
    for (int i = 0; i < n; i++) sz += (int)strlen(types[i]) + 1; // +1 para vírgula 
    char *sig = malloc(sz + 4);
    sig[0] = '('; sig[1] = '\0';
    for (int i = 0; i < n; i++) {
        if (i > 0) strcat(sig, ",");
        strcat(sig, types[i]);
    }
    strcat(sig, ")");
    return sig;
}

// Constrói a string de tipos de argumentos para mensagens de erro: "nome(t1,t2,...)"
static char *make_call_token(const char *name, char **types, int n) {
    int sz = (int)strlen(name) + 2;
    for (int i = 0; i < n; i++) sz += (int)strlen(types[i]) + 1;
    char *tok = malloc(sz + 4);
    sprintf(tok, "%s(", name);
    for (int i = 0; i < n; i++) {
        if (i > 0) strcat(tok, ",");
        strcat(tok, types[i]);
    }
    strcat(tok, ")");
    return tok;
}


//  Operações na tabela de símbolos local                              
static LocalSym *local_lookup(LocalSym *head, const char *name) {
    for (LocalSym *s = head; s; s = s->next)
        if (strcmp(s->name, name) == 0) return s;
    return NULL;
}

// Insere no fim; retuirn NULL se duplicado 
static LocalSym *local_insert(LocalSym **head_ptr, const char *name,
                               const char *type, int is_param,
                               int line, int col) {
    if (local_lookup(*head_ptr, name)) return NULL;
    LocalSym *s = malloc(sizeof(LocalSym));
    s->name     = strdup(name);
    s->type     = strdup(type);
    s->is_param = is_param;
    s->line     = line;
    s->col      = col;
    s->next     = NULL;
    // Adicionar no fim 
    if (!*head_ptr) { *head_ptr = s; return s; }
    LocalSym *cur = *head_ptr;
    while (cur->next) cur = cur->next;
    cur->next = s;
    return s;
}

//  Operações na tabela de símbolos da classe                          //
static ClassSym *class_lookup_field(ClassSym *head, const char *name) {
    for (ClassSym *s = head; s; s = s->next)
        if (!s->is_method && strcmp(s->name, name) == 0) return s;
    return NULL;
}

// Procura método pelo nome + tipos de parâmetros exactos 
static ClassSym *class_lookup_method_exact(ClassSym *head, const char *name,
                                            char **argtypes, int nargs) {
    for (ClassSym *s = head; s; s = s->next) {
        if (!s->is_method) continue;
        if (strcmp(s->name, name) != 0) continue;
        if (s->num_params != nargs) continue;
        int match = 1;
        for (int i = 0; i < nargs; i++) {
            if (strcmp(argtypes[i], s->param_types[i]) != 0) { match = 0; break; }
        }
        if (match) return s;
    }
    return NULL;
}

// Procura método com parâmetros compatíveis (alargamento int→double permitido) 
// Devolve: entrada encontrada ou NULL; define *ambiguous=1 se houver múltiplos compatíveis
static ClassSym *class_lookup_method(ClassSym *head, const char *name,
                                     char **argtypes, int nargs,
                                     int *ambiguous) {
    *ambiguous = 0;
    // Primeiro: correspondência exacta 
    ClassSym *exact = class_lookup_method_exact(head, name, argtypes, nargs);
    if (exact) return exact;

    // Correspondência compatível
    ClassSym *found = NULL;
    int count = 0;
    for (ClassSym *s = head; s; s = s->next) {
        if (!s->is_method) continue;
        if (strcmp(s->name, name) != 0) continue;
        if (s->num_params != nargs) continue;
        int compat = 1;
        for (int i = 0; i < nargs; i++) {
            const char *at = argtypes[i];
            const char *ft = s->param_types[i];
            /* undef não é compatível com nada */
            if (strcmp(at, T_UNDEF) == 0) { compat = 0; break; }
            /* Exacto ou alargamento int→double */
            if (!types_equal(at, ft) && !type_widens(at, ft)) { compat = 0; break; }
        }
        if (compat) { count++; found = s; }
    }
    if (count == 1) return found;
    if (count > 1)  { *ambiguous = 1; return NULL; }
    return NULL;
}

// Adiciona um ClassSym no fim da lista 
static void class_append(ClassSym **head_ptr, ClassSym *s) {
    if (!*head_ptr) { *head_ptr = s; return; }
    ClassSym *cur = *head_ptr;
    while (cur->next) cur = cur->next;
    cur->next = s;
}


// Processar uma declaração de field
static void process_field(Node *fdecl) {
    /* fdecl: nó FieldDecl: filho[0]=TipoNó, filho[1]=Identificador */
    if (!fdecl) return;
    Node *type_node = child(fdecl, 0);
    Node *id_node   = child(fdecl, 1);
    if (!type_node || !id_node) return;

    const char *type = kind_to_type(type_node->kind);
    const char *name = id_node->val;
    int line = id_node->line, col_  = id_node->col;

    // Verificar nome reservado 
    if (strcmp(name, "_") == 0) {
        printf("Line %d, col %d: Symbol _ is reserved\n", line, col_);
        semantic_errors++;
        return;
    }
    // Verificar campo duplicado
    if (class_lookup_field(g_class_table, name)) {
        printf("Line %d, col %d: Symbol %s already defined\n", line, col_, name);
        semantic_errors++;
        return;
    }
    // Adicionar à tabela de classe
    ClassSym *s = malloc(sizeof(ClassSym));
    s->name        = strdup(name);
    s->is_method   = 0;
    s->type        = strdup(type);
    s->param_sig   = NULL;
    s->param_types = NULL;
    s->num_params  = 0;
    s->locals      = NULL;
    s->line        = line;
    s->col         = col_;
    s->next        = NULL;
    class_append(&g_class_table, s);
}


// Processar uma declaração de método (assinatura + parâmetros)
static ClassSym *process_method_sig(Node *mdecl) {
    // mdecl: MethodDecl: filho[0]=MethodHeader, filho[1]=MethodBody 
    Node *header = child(mdecl, 0);
    if (!header) return NULL;

    Node *type_node   = child(header, 0); // Int/Double/Bool/Void 
    Node *name_node   = child(header, 1); // Identificador
    Node *params_node = child(header, 2); // MethodParams

    if (!type_node || !name_node) return NULL;

    const char *ret_type = kind_to_type(type_node->kind);
    const char *mname    = name_node->val;
    int mline = name_node->line, mcol = name_node->col;

    // Construir lista de parâmetros PRIMEIRO 
    int num_params = params_node ? params_node->nchildren : 0;
    char **ptypes  = num_params ? malloc(num_params * sizeof(char*)) : NULL;

    // Processar parâmetros numa tabela local nova
    LocalSym *locals = NULL;
    // Primeira entrada: return
    local_insert(&locals, "return", ret_type, 0, 0, 0);

    for (int i = 0; i < num_params; i++) {
        Node *pdecl     = child(params_node, i); // ParamDecl - declaração de parâmetro
        Node *ptype_node = child(pdecl, 0);
        Node *pid_node   = child(pdecl, 1);
        if (!ptype_node || !pid_node) { ptypes[i] = strdup(T_UNDEF); continue; }

        const char *ptype = kind_to_type(ptype_node->kind);
        const char *pname = pid_node->val;
        int pline = pid_node->line, pcol = pid_node->col;

        ptypes[i] = strdup(ptype);

        // Verificar nome reservado
        if (strcmp(pname, "_") == 0) {
            printf("Line %d, col %d: Symbol _ is reserved\n", pline, pcol);
            semantic_errors++;
            continue;
        }
        // Verificar parâmetro duplicado
        if (local_lookup(locals, pname)) {
            printf("Line %d, col %d: Symbol %s already defined\n", pline, pcol, pname);
            semantic_errors++;
            continue;
        }
        local_insert(&locals, pname, ptype, 1, pline, pcol);
    }

    char *psig = make_param_sig(ptypes, num_params);

    // Verifica se o nome do método é reservado (APÓS o processamento dos parâmetros, para que os erros de parâmetros apareçam primeiro)
    if (strcmp(mname, "_") == 0) {
        printf("Line %d, col %d: Symbol _ is reserved\n", mline, mcol);
        semantic_errors++;
        free(psig);
        for (int i = 0; i < num_params; i++) free(ptypes[i]);
        free(ptypes);
        // Free local list
        LocalSym *ls = locals;
        while (ls) { LocalSym *nx = ls->next; free(ls->name); free(ls->type); free(ls); ls = nx; }
        return NULL;
    }

    // Verificar método duplicado (mesmo nome + mesmos tipos de parâmetros)
    ClassSym *existing = class_lookup_method_exact(g_class_table, mname, ptypes, num_params);
    if (existing) {
        printf("Line %d, col %d: Symbol %s%s already defined\n", mline, mcol, mname, psig);
        semantic_errors++;
        // Libertar recursos, não adicionar
        free(psig);
        for (int i = 0; i < num_params; i++) free(ptypes[i]);
        free(ptypes);
        // Libertar lista local
        while (locals) { LocalSym *nx = locals->next; free(nx ? nx->name : NULL); /* skip */ locals = nx; }
        return NULL;
    }

    // Adicionar à tabela de classe
    ClassSym *s    = malloc(sizeof(ClassSym));
    s->name        = strdup(mname);
    s->is_method   = 1;
    s->type        = strdup(ret_type);
    s->param_sig   = psig;
    s->param_types = ptypes;
    s->num_params  = num_params;
    s->locals      = locals;
    s->line        = mline;
    s->col         = mcol;
    s->next        = NULL;
    class_append(&g_class_table, s);
    return s;
}

// Verificador de tipos de expressões (devolve tipo; anota n->type) 

// Declarações antecipadas
static const char *check_expr(Node *n, ClassSym *meth);
static void check_stmt(Node *n, ClassSym *meth);

static const char *set_type(Node *n, const char *t) {
    if (!n) return t;
    free(n->type);
    n->type = strdup(t);
    return t;
}

static const char *check_expr(Node *n, ClassSym *meth) {
    if (!n) return T_UNDEF;

    // Literais
    if (strcmp(n->kind, "Natural") == 0) {
        if (natural_out_of_bounds(n->val)) {
            printf("Line %d, col %d: Number %s out of bounds\n",
                   n->line, n->col, n->val);
            semantic_errors++;
        }
        return set_type(n, T_INT);
    }
    if (strcmp(n->kind, "Decimal") == 0) {
        if (decimal_out_of_bounds(n->val)) {
            printf("Line %d, col %d: Number %s out of bounds\n",
                   n->line, n->col, n->val);
            semantic_errors++;
        }
        return set_type(n, T_DOUBLE);
    }
    if (strcmp(n->kind, "BoolLit") == 0) return set_type(n, T_BOOL);

    // Pesquisa de identificador 
    if (strcmp(n->kind, "Identifier") == 0) {
        const char *name = n->val;
        // Pesquisar primeiro na tabela local
        if (meth && meth->locals) {
            LocalSym *ls = local_lookup(meth->locals, name);
            if (ls) return set_type(n, ls->type);
        }
        // Depois na tabela de classe (campos)
        ClassSym *cs = class_lookup_field(g_class_table, name);
        if (cs) return set_type(n, cs->type);
        // Não encontrado
        printf("Line %d, col %d: Cannot find symbol %s\n",
               n->line, n->col, name);
        semantic_errors++;
        return set_type(n, T_UNDEF);
    }

    // Operadores unários
    if (strcmp(n->kind, "Not") == 0) {
        const char *t = check_expr(child(n,0), meth);
        if (strcmp(t, T_BOOL) != 0) {
            printf("Line %d, col %d: Operator ! cannot be applied to type %s\n",
                   n->line, n->col, t);
            semantic_errors++;
        }
        return set_type(n, T_BOOL);  // sempre booleano
    }
    if (strcmp(n->kind, "Minus") == 0 || strcmp(n->kind, "Plus") == 0) {
        const char *t = check_expr(child(n,0), meth);
        if (!is_numeric(t)) {
            char op = (strcmp(n->kind,"Minus")==0) ? '-' : '+';
            printf("Line %d, col %d: Operator %c cannot be applied to type %s\n",
                   n->line, n->col, op, t);
            semantic_errors++;
            return set_type(n, T_UNDEF);
        }
        return set_type(n, t);
    }

    // Aritmética binária: +, -, *, /, %
    if (strcmp(n->kind,"Add")==0 || strcmp(n->kind,"Sub")==0 ||
        strcmp(n->kind,"Mul")==0 || strcmp(n->kind,"Div")==0 ||
        strcmp(n->kind,"Mod")==0) {
        const char *lt = check_expr(child(n,0), meth);
        const char *rt = check_expr(child(n,1), meth);
        const char *res = arith_result(lt, rt);
        if (strcmp(res, T_UNDEF) == 0) {
            // nome do operador para mensagem de erro
            const char *op = strcmp(n->kind,"Add")==0 ? "+" :
                             strcmp(n->kind,"Sub")==0 ? "-" :
                             strcmp(n->kind,"Mul")==0 ? "*" :
                             strcmp(n->kind,"Div")==0 ? "/" : "%";
            printf("Line %d, col %d: Operator %s cannot be applied to types %s, %s\n",
                   n->line, n->col, op, lt, rt);
            semantic_errors++;
        }
        return set_type(n, res);
    }

    // AND, OR lógico/bit a bit
    if (strcmp(n->kind,"And")==0 || strcmp(n->kind,"Or")==0) {
        const char *lt = check_expr(child(n,0), meth);
        const char *rt = check_expr(child(n,1), meth);
        const char *op = strcmp(n->kind,"And")==0 ? "&&" : "||";
        if (strcmp(lt,T_BOOL)!=0 || strcmp(rt,T_BOOL)!=0) {
            printf("Line %d, col %d: Operator %s cannot be applied to types %s, %s\n",
                   n->line, n->col, op, lt, rt);
            semantic_errors++;
        }
        return set_type(n, T_BOOL);  // sempre booleano
    }
    
    // XOR lógico/bit a bit: int^int→int, bool^bool→bool, senão erro
    if (strcmp(n->kind,"Xor")==0) {
        const char *lt = check_expr(child(n,0), meth);
        const char *rt = check_expr(child(n,1), meth);
        if (strcmp(lt,T_INT)==0 && strcmp(rt,T_INT)==0)
            return set_type(n, T_INT);
        if (strcmp(lt,T_BOOL)==0 && strcmp(rt,T_BOOL)==0)
            return set_type(n, T_BOOL);
        printf("Line %d, col %d: Operator ^ cannot be applied to types %s, %s\n",
               n->line, n->col, lt, rt);
        semantic_errors++;
        return set_type(n, T_UNDEF);
    }

    // Deslocamento: int<<int→int, senão erro 
    if (strcmp(n->kind,"Lshift")==0 || strcmp(n->kind,"Rshift")==0) {
        const char *lt = check_expr(child(n,0), meth);
        const char *rt = check_expr(child(n,1), meth);
        const char *op = strcmp(n->kind,"Lshift")==0 ? "<<" : ">>";
        if (strcmp(lt,T_INT)!=0 || strcmp(rt,T_INT)!=0) {
            printf("Line %d, col %d: Operator %s cannot be applied to types %s, %s\n",
                   n->line, n->col, op, lt, rt);
            semantic_errors++;
            return set_type(n, T_UNDEF);
        }
        return set_type(n, T_INT);
    }

    //  Comparação: ==, != 
    if (strcmp(n->kind,"Eq")==0 || strcmp(n->kind,"Ne")==0) {
        const char *lt = check_expr(child(n,0), meth);
        const char *rt = check_expr(child(n,1), meth);
        const char *op = strcmp(n->kind,"Eq")==0 ? "==" : "!=";
        // Válido: ambos numéricos (mistura int/double permitida), ou ambos booleanos
        int ok = (is_numeric(lt) && is_numeric(rt)) ||
                 (strcmp(lt,T_BOOL)==0 && strcmp(rt,T_BOOL)==0);
        if (!ok) {
            printf("Line %d, col %d: Operator %s cannot be applied to types %s, %s\n",
                   n->line, n->col, op, lt, rt);
            semantic_errors++;
        }
        return set_type(n, T_BOOL);  // sempre booleano
    }

    //  Relacionais: <, >, <=, >=
    if (strcmp(n->kind,"Lt")==0 || strcmp(n->kind,"Gt")==0 ||
        strcmp(n->kind,"Le")==0 || strcmp(n->kind,"Ge")==0) {
        const char *lt = check_expr(child(n,0), meth);
        const char *rt = check_expr(child(n,1), meth);
        const char *op = strcmp(n->kind,"Lt")==0 ? "<" :
                         strcmp(n->kind,"Gt")==0 ? ">" :
                         strcmp(n->kind,"Le")==0 ? "<=" : ">=";
        if (!is_numeric(lt) || !is_numeric(rt)) {
            printf("Line %d, col %d: Operator %s cannot be applied to types %s, %s\n",
                   n->line, n->col, op, lt, rt);
            semantic_errors++;
        }
        return set_type(n, T_BOOL);  // sempre booleano
    }

    //  Atribuição: filho[0]=Identificador(LHS), filho[1]=RHS
    if (strcmp(n->kind,"Assign")==0) {
        Node *lhs = child(n,0); // Identificador
        Node *rhs = child(n,1);
        const char *lt = check_expr(lhs, meth);
        const char *rt = check_expr(rhs, meth);
        // Reportar erro se não for compatível
        if (!type_widens(rt, lt)) {
            printf("Line %d, col %d: Operator = cannot be applied to types %s, %s\n",
                   n->line, n->col, lt, rt);
            semantic_errors++;
        }
        return set_type(n, lt);
    }

    // Comprimento (.length): filho[0]=Identificador
    if (strcmp(n->kind,"Length")==0) {
        Node *id = child(n,0);
        const char *t = check_expr(id, meth);
        if (strcmp(t, T_STRARRAY) != 0) {
            printf("Line %d, col %d: Operator .length cannot be applied to type %s\n",
                   n->line, n->col, t);
            semantic_errors++;
        }
        return set_type(n, T_INT);
    }

    // ParseArgs: filho[0]=Identificador(array), filho[1]=índice
    if (strcmp(n->kind,"ParseArgs")==0) {
        Node *arr_id = child(n,0);
        Node *idx    = child(n,1);
        const char *at = arr_id ? check_expr(arr_id, meth) : T_UNDEF;
        const char *it = idx    ? check_expr(idx,    meth) : T_UNDEF;
        if (strcmp(at, T_STRARRAY) != 0 || strcmp(it, T_INT) != 0) {
            printf("Line %d, col %d: Operator Integer.parseInt cannot be applied to types %s, %s\n",
                   n->line, n->col, at, it);
            semantic_errors++;
        }
        return set_type(n, T_INT);
    }

    // Chamada: filho[0]=Identificador(nome), filho[1..n]=argumentos
    if (strcmp(n->kind,"Call")==0) {
        Node *id   = child(n,0);
        int nargs  = n->nchildren - 1;
        char **argtypes = nargs > 0 ? malloc(nargs * sizeof(char*)) : NULL;

        // Verificar tipos dos argumentos
        for (int i = 0; i < nargs; i++)
            argtypes[i] = strdup(check_expr(child(n, i+1), meth));

        const char *mname = id ? id->val : "?";
        int ambig = 0;
        ClassSym *found = class_lookup_method(g_class_table, mname,
                                              argtypes, nargs, &ambig);
        const char *res;
        if (found) {
            // Anotar filho Identificador com assinatura de parâmetros
            set_type(id, found->param_sig);
            res = set_type(n, found->type);
        } else {
            char *tok = make_call_token(mname, argtypes, nargs);
            if (ambig)
                printf("Line %d, col %d: Reference to method %s is ambiguous\n",
                       id ? id->line : n->line, id ? id->col : n->col, tok);
            else
                printf("Line %d, col %d: Cannot find symbol %s\n",
                       id ? id->line : n->line, id ? id->col : n->col, tok);
            free(tok);
            semantic_errors++;
            set_type(id, T_UNDEF);
            res = set_type(n, T_UNDEF);
        }
        for (int i = 0; i < nargs; i++) free(argtypes[i]);
        free(argtypes);
        return res;
    }

    return T_UNDEF;
}


//  Verificador de instruções 
static void check_stmt(Node *n, ClassSym *meth) {
    if (!n) return;

    if (strcmp(n->kind,"If")==0) {
        Node *cond = child(n,0);
        const char *ct = check_expr(cond, meth);
        check_stmt(child(n,1), meth);
        check_stmt(child(n,2), meth);
        if (strcmp(ct, T_BOOL) != 0) {
            Node *root = cond ? cond : n;
            printf("Line %d, col %d: Incompatible type %s in if statement\n",
                   root->line, root->col, ct);
            semantic_errors++;
        }
        return;
    }

    if (strcmp(n->kind,"While")==0) {
        Node *cond = child(n,0);
        const char *ct = check_expr(cond, meth);
        check_stmt(child(n,1), meth);
        if (strcmp(ct, T_BOOL) != 0) {
            Node *root = cond ? cond : n;
            printf("Line %d, col %d: Incompatible type %s in while statement\n",
                   root->line, root->col, ct);
            semantic_errors++;
        }
        return;
    }

    if (strcmp(n->kind,"Return")==0) {
        // Obter tipo de retorno do método
        const char *ret_type = T_VOID;
        if (meth && meth->locals) {
            LocalSym *rs = local_lookup(meth->locals, "return");
            if (rs) ret_type = rs->type;
        }
        Node *expr = child(n,0);
        if (!expr) {
            // return sem valor
            if (strcmp(ret_type, T_VOID) != 0) {
                printf("Line %d, col %d: Incompatible type void in return statement\n",
                       n->line, n->col);
                semantic_errors++;
            }
        } else {
            const char *et = check_expr(expr, meth);
            if (strcmp(ret_type, T_VOID) == 0) {
                // void method can't return expression
                Node *root = expr;
                printf("Line %d, col %d: Incompatible type %s in return statement\n",
                       root->line, root->col, et);
                semantic_errors++;
            } else if (!type_widens(et, ret_type)) {
                Node *root = expr; // nó de erro
                printf("Line %d, col %d: Incompatible type %s in return statement\n",
                       root->line, root->col, et);
                semantic_errors++;
            }
        }
        return;
    }

    if (strcmp(n->kind,"Print")==0) {
        Node *arg = child(n,0);
        if (arg && strcmp(arg->kind,"StrLit")==0) {
            set_type(arg, "String");  // anotar StrLit
        } else if (arg) {
            const char *at = check_expr(arg, meth);
            if (strcmp(at, T_VOID) == 0 || strcmp(at, T_UNDEF) == 0 || strcmp(at, T_STRARRAY) == 0) {
                printf("Line %d, col %d: Incompatible type %s in System.out.print statement\n",
                       arg->line, arg->col, at);
                semantic_errors++;
            }
        }
        return;
    }

    if (strcmp(n->kind,"Block")==0) {
        for (int i = 0; i < n->nchildren; i++)
            check_stmt(n->children[i], meth);
        return;
    }

    // Assign, Call ou ParseArgs usados como instrução
    if (strcmp(n->kind,"Assign")==0 || strcmp(n->kind,"Call")==0 ||
        strcmp(n->kind,"ParseArgs")==0) {
        check_expr(n, meth);
        return;
    }
}

//  Fase 2: Processar o corpo do método
static void process_method_body(ClassSym *ms, Node *mdecl) {
    Node *body = child(mdecl, 1); // MethodBody - corpo do método
    if (!body) return;

    for (int i = 0; i < body->nchildren; i++) {
        Node *stmt = body->children[i];
        if (!stmt) continue;

        if (strcmp(stmt->kind,"VarDecl")==0) {
            // filho[0]=TipoNó, filho[1]=Identificador
            Node *type_node = child(stmt,0);
            Node *id_node   = child(stmt,1);
            if (!type_node || !id_node) continue;
            const char *type = kind_to_type(type_node->kind);
            const char *name = id_node->val;
            int line = id_node->line, col_ = id_node->col;

            if (strcmp(name,"_")==0) {
                printf("Line %d, col %d: Symbol _ is reserved\n", line, col_);
                semantic_errors++;
                continue;
            }
            if (!local_insert(&ms->locals, name, type, 0, line, col_)) {
                printf("Line %d, col %d: Symbol %s already defined\n", line, col_, name);
                semantic_errors++;
            }
        } else {
            check_stmt(stmt, ms);
        }
    }
}

//  Imprimir tabelas de símbolos
static void print_class_table(void) {
    printf("===== Class %s Symbol Table =====\n", g_class_name ? g_class_name : "?");
    for (ClassSym *s = g_class_table; s; s = s->next) {
        if (s->is_method)
            printf("%s\t%s\t%s\n", s->name, s->param_sig, s->type);
        else
            printf("%s\t\t%s\n", s->name, s->type);
    }
}

static void print_method_table(ClassSym *ms) {
    printf("===== Method %s%s Symbol Table =====\n",
           ms->name, ms->param_sig);
    for (LocalSym *ls = ms->locals; ls; ls = ls->next) {
        if (ls->is_param)
            printf("%s\t\t%s\tparam\n", ls->name, ls->type);
        else
            printf("%s\t\t%s\n", ls->name, ls->type);
    }
}

//  Ponto de entrada: do_semantic
void do_semantic(Node *root, int print_output) {
    if (!root) return;
    semantic_errors = 0;

    // Obter nome da classe
    Node *class_id = child(root, 0);
    g_class_name = class_id ? class_id->val : NULL;

    // ---- Fase 1: Processar todos os membros da classe por ordem de declaração (intercalado) ----
    int n_methods = 0;
    for (int i = 1; i < root->nchildren; i++) {
        Node *member = root->children[i];
        if (member && strcmp(member->kind,"MethodDecl")==0) n_methods++;
    }
    ClassSym **method_syms = n_methods ? malloc(n_methods*sizeof(ClassSym*)) : NULL;
    Node    **method_decls = n_methods ? malloc(n_methods*sizeof(Node*))     : NULL;
    int mi = 0;
    for (int i = 1; i < root->nchildren; i++) {
        Node *member = root->children[i];
        if (!member) continue;
        if (strcmp(member->kind,"FieldDecl")==0)
            process_field(member);
        else if (strcmp(member->kind,"MethodDecl")==0) {
            method_syms[mi] = process_method_sig(member);
            method_decls[mi] = member;
            mi++;
        }
    }

    // ---- Fase 2: Corpos dos métodos ----
    for (int i = 0; i < n_methods; i++) {
        if (method_syms[i])
            process_method_body(method_syms[i], method_decls[i]);
    }

    // ---- Output ----
    if (print_output) {
        // Tabela de classe
        print_class_table();

        // Tabelas de métodos
        for (int i = 0; i < n_methods; i++) {
            if (method_syms[i]) {
                printf("\n");
                print_method_table(method_syms[i]);
            }
        }
        // Blank line before AST
        printf("\n");
        // Annotated AST
        printAST(root, 0);
    }

    free(method_syms);
    free(method_decls);
}
