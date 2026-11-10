// codegen.c – Geração de código LLVM IR para a linguagem JUC
// Diogo Henriques – 2019222685

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "ast.h"
#include "semantics.h"
#include "codegen.h"

// ---- Estado global do gerador ----
static int  g_tmp;          // contador de registos temporários (%rN)
static int  g_lbl;          // contador de etiquetas básicas (global, não repõe por função)
static int  g_term;         // 1 se o bloco atual já tem terminador (ret/br)
static char g_cur_blk[256]; // nome do bloco básico atual

// ---- Tabela de strings literais ----
#define MAX_STRS 2048
static const char *g_str_val[MAX_STRS];  // conteúdo do val do nó StrLit
static int         g_str_blen[MAX_STRS]; // comprimento em bytes (sem '\0')
static int         g_nstrs;

// ---- Acesso a filho da AST ----
static Node *ch(Node *n, int i) {
    if (!n || i < 0 || i >= n->nchildren) return NULL;
    return n->children[i];
}

// ---- Procura símbolo local pelo nome ----
static LocalSym *loc_find(LocalSym *head, const char *name) {
    for (LocalSym *s = head; s; s = s->next)
        if (strcmp(s->name, name) == 0) return s;
    return NULL;
}

// Verifica se um nome é local ao método E tem o tipo anotado correspondente.
// Necessário para distinguir referências a campos globais vs variáveis locais
// que partilham o mesmo nome (shadowing).
static int is_local_typed(ClassSym *meth, const char *name, const char *type) {
    if (!meth) return 0;
    LocalSym *ls = loc_find(meth->locals, name);
    if (!ls) return 0;
    if (!type) return 1;
    return strcmp(ls->type, type) == 0;
}

// ---- Mapeamento de tipo JUC → tipo LLVM IR ----
static const char *ll_type(const char *t) {
    if (!t)                          return "i32";
    if (strcmp(t, "int")      == 0) return "i32";
    if (strcmp(t, "double")   == 0) return "double";
    if (strcmp(t, "boolean")  == 0) return "i1";
    if (strcmp(t, "void")     == 0) return "void";
    return "i32";
}

// Valor zero para inicialização de globais
static const char *ll_zero(const char *t) {
    if (strcmp(t, "double")  == 0) return "0.0";
    if (strcmp(t, "boolean") == 0) return "false";
    return "0";
}

// Converte kind do nó de tipo → string de tipo JUC
static const char *kind2type(const char *kind) {
    if (!kind) return "undef";
    if (strcmp(kind, "Int")         == 0) return "int";
    if (strcmp(kind, "Double")      == 0) return "double";
    if (strcmp(kind, "Bool")        == 0) return "boolean";
    if (strcmp(kind, "Void")        == 0) return "void";
    if (strcmp(kind, "StringArray") == 0) return "String[]";
    return "undef";
}

// Constrói param_sig a partir de um array de tipos
static char *build_psig(char **types, int n) {
    int sz = 3;
    for (int i = 0; i < n; i++) sz += (int)strlen(types[i]) + 1;
    char *sig = malloc(sz);
    sig[0] = '('; sig[1] = '\0';
    for (int i = 0; i < n; i++) {
        if (i > 0) strcat(sig, ",");
        strcat(sig, types[i]);
    }
    strcat(sig, ")");
    return sig;
}

// ---- Mangling do nome do método ----
// ex: nome="factorial", psig="(int)" → buf="_factorial_int"
static void mangle(char *buf, size_t sz, const char *mname, const char *psig) {
    snprintf(buf, sz, "_%s", mname);
    if (!psig || strcmp(psig, "()") == 0) return;
    const char *p = psig + 1; // saltar '('
    while (*p && *p != ')') {
        const char *start = p;
        while (*p && *p != ',' && *p != ')') p++;
        int tlen = (int)(p - start);
        char tname[64] = {0};
        strncpy(tname, start, tlen < 63 ? tlen : 63);
        size_t cur = strlen(buf);
        strncat(buf + cur, "_", sz - cur - 1);
        cur = strlen(buf);
        if      (strcmp(tname, "int")      == 0) strncat(buf + cur, "int",         sz - cur - 1);
        else if (strcmp(tname, "double")   == 0) strncat(buf + cur, "double",      sz - cur - 1);
        else if (strcmp(tname, "boolean")  == 0) strncat(buf + cur, "boolean",     sz - cur - 1);
        else if (strcmp(tname, "String[]") == 0) strncat(buf + cur, "StringArray", sz - cur - 1);
        if (*p == ',') p++;
    }
}

// ---- Procura ClassSym por nome + param_sig ----
static ClassSym *find_msym(const char *mname, const char *psig) {
    for (ClassSym *s = g_class_table; s; s = s->next) {
        if (s->is_method
            && strcmp(s->name, mname) == 0
            && psig && strcmp(s->param_sig, psig) == 0)
            return s;
    }
    return NULL;
}

// ---- Conversão de string escape (lexer) para hex LLVM IR ----
// O val do nó StrLit é "conteúdo" (com aspas externas incluídas).
// Esta função processa apenas o conteúdo (entre as aspas) e converte
// as sequências de escape para bytes reais em formato hex LLVM.
// Retorna comprimento em bytes (sem '\0').
static int str_to_llvm(const char *val, char *out, int outsz) {
    // Saltar a aspa de abertura
    if (val && val[0] == '"') val++;
    int len = 0;
    char *p = out;
    // Processar até à aspa de fecho (ou fim da string)
    while (*val && *val != '"') {
        unsigned char c;
        if (*val == '\\' && *(val + 1)) {
            val++;
            switch (*val) {
                case 'n':  c = 0x0A; break;
                case 't':  c = 0x09; break;
                case 'r':  c = 0x0D; break;
                case 'f':  c = 0x0C; break;
                case '\\': c = 0x5C; break;
                case '"':  c = 0x22; break;
                default:   c = (unsigned char)*val; break;
            }
            val++;
        } else {
            c = (unsigned char)*val;
            val++;
        }
        // Escapar caracteres não imprimíveis, aspas e barras invertidas
        if (c < 0x20 || c >= 0x7F || c == 0x22 || c == 0x5C) {
            p += snprintf(p, outsz - (p - out), "\\%02X", (unsigned)c);
        } else {
            *p++ = (char)c;
        }
        len++;
    }
    snprintf(p, outsz - (p - out), "\\00");
    return len;
}

// ---- Pré-varredura da AST para recolher strings literais ----
static void scan_strs(Node *n) {
    if (!n) return;
    if (strcmp(n->kind, "StrLit") == 0 && n->val) {
        for (int i = 0; i < g_nstrs; i++)
            if (strcmp(g_str_val[i], n->val) == 0) return; // já existe
        if (g_nstrs < MAX_STRS) {
            char tmp[16384];
            int blen = str_to_llvm(n->val, tmp, sizeof(tmp));
            g_str_val[g_nstrs]  = n->val;
            g_str_blen[g_nstrs] = blen;
            g_nstrs++;
        }
    }
    for (int i = 0; i < n->nchildren; i++) scan_strs(n->children[i]);
}

// Devolve índice de uma string literal na tabela
static int find_str(const char *val) {
    for (int i = 0; i < g_nstrs; i++)
        if (strcmp(g_str_val[i], val) == 0) return i;
    return 0;
}

// ---- Emissão do valor double em hexadecimal IEEE 754 LLVM ----
static void emit_dbl(double v) {
    union { double d; uint64_t u; } x;
    x.d = v;
    printf("0x%016llX", (unsigned long long)x.u);
}

// ---- Remove underscores de literais numéricos ----
static void strip_us(const char *s, char *out) {
    while (*s) {
        if (*s != '_') *out++ = *s;
        s++;
    }
    *out = '\0';
}

// ---- Alargamento de tipo int → double quando necessário ----
static int widen(int reg, const char *from, const char *to) {
    if (!from || !to || strcmp(from, to) == 0) return reg;
    if (strcmp(from, "int") == 0 && strcmp(to, "double") == 0) {
        int r = g_tmp++;
        printf("  %%r%d = sitofp i32 %%r%d to double\n", r, reg);
        return r;
    }
    return reg;
}

// ---- Emissão de getelementptr para string literal ----
static int emit_strptr(int idx) {
    int blen = g_str_blen[idx] + 1; // +1 para '\0'
    int r = g_tmp++;
    printf("  %%r%d = getelementptr [%d x i8], [%d x i8]* @.str%d, i32 0, i32 0\n",
           r, blen, blen, idx);
    return r;
}

// ---- Emissão de getelementptr para string de formato ----
static int emit_fmtptr(const char *fname, int blen) {
    int r = g_tmp++;
    printf("  %%r%d = getelementptr [%d x i8], [%d x i8]* @.%s, i32 0, i32 0\n",
           r, blen, blen, fname);
    return r;
}

// ---- Declarações de funções internas (forward) ----
static int  cg_expr(Node *n, ClassSym *meth);
static void cg_stmt(Node *n, ClassSym *meth);

// ---- Emissão de load de variável (local ou global) ----
static int emit_load(const char *name, const char *type, ClassSym *meth) {
    int r = g_tmp++;
    const char *lt = ll_type(type);
    if (is_local_typed(meth, name, type))
        printf("  %%r%d = load %s, %s* %%local_%s\n", r, lt, lt, name);
    else
        printf("  %%r%d = load %s, %s* @%s\n", r, lt, lt, name);
    return r;
}

// ---- Emissão de store de variável (local ou global) ----
static void emit_store(const char *name, const char *type, int reg, ClassSym *meth) {
    const char *lt = ll_type(type);
    if (is_local_typed(meth, name, type))
        printf("  store %s %%r%d, %s* %%local_%s\n", lt, reg, lt, name);
    else
        printf("  store %s %%r%d, %s* @%s\n", lt, reg, lt, name);
}

// ---- Emissão de chamada a printf para um valor ----
static void emit_print_val(int val_reg, const char *type) {
    if (strcmp(type, "int") == 0) {
        int fmt = emit_fmtptr("fmt_int", 3);
        printf("  call i32 (i8*, ...) @printf(i8* %%r%d, i32 %%r%d)\n", fmt, val_reg);
    } else if (strcmp(type, "double") == 0) {
        int fmt = emit_fmtptr("fmt_double", 6);
        printf("  call i32 (i8*, ...) @printf(i8* %%r%d, double %%r%d)\n", fmt, val_reg);
    } else if (strcmp(type, "boolean") == 0) {
        // Selecionar "true" ou "false" com select
        int tr = g_tmp++;
        printf("  %%r%d = getelementptr [5 x i8], [5 x i8]* @.str_true,  i32 0, i32 0\n", tr);
        int fr = g_tmp++;
        printf("  %%r%d = getelementptr [6 x i8], [6 x i8]* @.str_false, i32 0, i32 0\n", fr);
        int sel = g_tmp++;
        printf("  %%r%d = select i1 %%r%d, i8* %%r%d, i8* %%r%d\n", sel, val_reg, tr, fr);
        int fmt = emit_fmtptr("fmt_str", 3);
        printf("  call i32 (i8*, ...) @printf(i8* %%r%d, i8* %%r%d)\n", fmt, sel);
    }
}

// ====================================================================
// Geração de código para expressões
// ====================================================================
static int cg_expr(Node *n, ClassSym *meth) {
    if (!n) return 0;

    // --- Natural (literal inteiro) ---
    if (strcmp(n->kind, "Natural") == 0) {
        char clean[64]; strip_us(n->val, clean);
        int r = g_tmp++;
        printf("  %%r%d = add i32 0, %s\n", r, clean);
        return r;
    }

    // --- Decimal (literal double) ---
    if (strcmp(n->kind, "Decimal") == 0) {
        char clean[128]; strip_us(n->val, clean);
        double dv = strtod(clean, NULL);
        int r = g_tmp++;
        printf("  %%r%d = fadd double 0.0, ", r);
        emit_dbl(dv);
        printf("\n");
        return r;
    }

    // --- BoolLit (literal booleano) ---
    if (strcmp(n->kind, "BoolLit") == 0) {
        int v = (strcmp(n->val, "true") == 0) ? 1 : 0;
        int r = g_tmp++;
        printf("  %%r%d = add i1 0, %d\n", r, v);
        return r;
    }

    // --- Identifier (referência a variável) ---
    if (strcmp(n->kind, "Identifier") == 0) {
        const char *type = n->type ? n->type : "int";
        return emit_load(n->val, type, meth);
    }

    // --- Not (!) ---
    if (strcmp(n->kind, "Not") == 0) {
        int v = cg_expr(ch(n, 0), meth);
        int r = g_tmp++;
        printf("  %%r%d = xor i1 %%r%d, true\n", r, v);
        return r;
    }

    // --- Minus unário (-) ---
    if (strcmp(n->kind, "Minus") == 0) {
        Node *c0 = ch(n, 0);
        int v = cg_expr(c0, meth);
        int r = g_tmp++;
        if (n->type && strcmp(n->type, "double") == 0)
            printf("  %%r%d = fsub double -0.0, %%r%d\n", r, v);
        else
            printf("  %%r%d = sub i32 0, %%r%d\n", r, v);
        return r;
    }

    // --- Plus unário (+) ---
    if (strcmp(n->kind, "Plus") == 0) {
        Node *c0 = ch(n, 0);
        int v = cg_expr(c0, meth);
        // Alargar se necessário (ex: +int quando tipo resultado é double)
        return widen(v, c0->type, n->type);
    }

    // --- Operações aritméticas binárias ---
    if (strcmp(n->kind,"Add")==0 || strcmp(n->kind,"Sub")==0 ||
        strcmp(n->kind,"Mul")==0 || strcmp(n->kind,"Div")==0 ||
        strcmp(n->kind,"Mod")==0) {
        Node *lnd = ch(n, 0), *rnd = ch(n, 1);
        int lv = cg_expr(lnd, meth);
        int rv = cg_expr(rnd, meth);
        const char *res_t = n->type;
        lv = widen(lv, lnd->type, res_t);
        rv = widen(rv, rnd->type, res_t);
        int r = g_tmp++;
        if (res_t && strcmp(res_t, "double") == 0) {
            const char *op = strcmp(n->kind,"Add")==0 ? "fadd" :
                             strcmp(n->kind,"Sub")==0 ? "fsub" :
                             strcmp(n->kind,"Mul")==0 ? "fmul" :
                             strcmp(n->kind,"Div")==0 ? "fdiv" : "frem";
            printf("  %%r%d = %s double %%r%d, %%r%d\n", r, op, lv, rv);
        } else {
            const char *op = strcmp(n->kind,"Add")==0 ? "add"  :
                             strcmp(n->kind,"Sub")==0 ? "sub"  :
                             strcmp(n->kind,"Mul")==0 ? "mul"  :
                             strcmp(n->kind,"Div")==0 ? "sdiv" : "srem";
            printf("  %%r%d = %s i32 %%r%d, %%r%d\n", r, op, lv, rv);
        }
        return r;
    }

    // --- Xor (^) ---
    if (strcmp(n->kind, "Xor") == 0) {
        int lv = cg_expr(ch(n, 0), meth);
        int rv = cg_expr(ch(n, 1), meth);
        int r  = g_tmp++;
        if (n->type && strcmp(n->type, "int") == 0)
            printf("  %%r%d = xor i32 %%r%d, %%r%d\n", r, lv, rv);
        else
            printf("  %%r%d = xor i1 %%r%d, %%r%d\n",  r, lv, rv);
        return r;
    }

    // --- Lshift (<<) / Rshift (>>) ---
    if (strcmp(n->kind, "Lshift") == 0 || strcmp(n->kind, "Rshift") == 0) {
        int lv = cg_expr(ch(n, 0), meth);
        int rv = cg_expr(ch(n, 1), meth);
        int r  = g_tmp++;
        const char *op = strcmp(n->kind, "Lshift") == 0 ? "shl" : "ashr";
        printf("  %%r%d = %s i32 %%r%d, %%r%d\n", r, op, lv, rv);
        return r;
    }

    // --- And (&&) com avaliação em curto-circuito ---
    if (strcmp(n->kind, "And") == 0) {
        int lbl = g_lbl++;
        // Gerar LHS; guardar bloco onde o branch fica
        int lv = cg_expr(ch(n, 0), meth);
        char lhs_blk[256]; snprintf(lhs_blk, sizeof(lhs_blk), "%s", g_cur_blk);
        if (!g_term)
            printf("  br i1 %%r%d, label %%and_rhs%d, label %%and_end%d\n",
                   lv, lbl, lbl);
        g_term = 0;
        // Bloco RHS
        printf("and_rhs%d:\n", lbl);
        snprintf(g_cur_blk, sizeof(g_cur_blk), "and_rhs%d", lbl);
        int rv = cg_expr(ch(n, 1), meth);
        char rhs_blk[256]; snprintf(rhs_blk, sizeof(rhs_blk), "%s", g_cur_blk);
        if (!g_term)
            printf("  br label %%and_end%d\n", lbl);
        g_term = 0;
        // Bloco de fim com PHI
        printf("and_end%d:\n", lbl);
        snprintf(g_cur_blk, sizeof(g_cur_blk), "and_end%d", lbl);
        int r = g_tmp++;
        printf("  %%r%d = phi i1 [ false, %%%s ], [ %%r%d, %%%s ]\n",
               r, lhs_blk, rv, rhs_blk);
        return r;
    }

    // --- Or (||) com avaliação em curto-circuito ---
    if (strcmp(n->kind, "Or") == 0) {
        int lbl = g_lbl++;
        int lv = cg_expr(ch(n, 0), meth);
        char lhs_blk[256]; snprintf(lhs_blk, sizeof(lhs_blk), "%s", g_cur_blk);
        if (!g_term)
            printf("  br i1 %%r%d, label %%or_end%d, label %%or_rhs%d\n",
                   lv, lbl, lbl);
        g_term = 0;
        // Bloco RHS
        printf("or_rhs%d:\n", lbl);
        snprintf(g_cur_blk, sizeof(g_cur_blk), "or_rhs%d", lbl);
        int rv = cg_expr(ch(n, 1), meth);
        char rhs_blk[256]; snprintf(rhs_blk, sizeof(rhs_blk), "%s", g_cur_blk);
        if (!g_term)
            printf("  br label %%or_end%d\n", lbl);
        g_term = 0;
        // Bloco de fim com PHI
        printf("or_end%d:\n", lbl);
        snprintf(g_cur_blk, sizeof(g_cur_blk), "or_end%d", lbl);
        int r = g_tmp++;
        printf("  %%r%d = phi i1 [ true, %%%s ], [ %%r%d, %%%s ]\n",
               r, lhs_blk, rv, rhs_blk);
        return r;
    }

    // --- Comparações: ==, !=, <, >, <=, >= ---
    if (strcmp(n->kind,"Eq")==0 || strcmp(n->kind,"Ne")==0 ||
        strcmp(n->kind,"Lt")==0 || strcmp(n->kind,"Gt")==0 ||
        strcmp(n->kind,"Le")==0 || strcmp(n->kind,"Ge")==0) {
        Node *lnd = ch(n, 0), *rnd = ch(n, 1);
        int lv = cg_expr(lnd, meth);
        int rv = cg_expr(rnd, meth);
        const char *lt = lnd->type ? lnd->type : "int";
        const char *rt = rnd->type ? rnd->type : "int";
        int use_fp   = (strcmp(lt,"double")==0 || strcmp(rt,"double")==0);
        int use_bool = (!use_fp && strcmp(lt,"boolean")==0);
        if (use_fp) {
            lv = widen(lv, lt, "double");
            rv = widen(rv, rt, "double");
        }
        int r = g_tmp++;
        if (use_fp) {
            const char *op = strcmp(n->kind,"Eq")==0 ? "oeq" :
                             strcmp(n->kind,"Ne")==0 ? "one" :
                             strcmp(n->kind,"Lt")==0 ? "olt" :
                             strcmp(n->kind,"Gt")==0 ? "ogt" :
                             strcmp(n->kind,"Le")==0 ? "ole" : "oge";
            printf("  %%r%d = fcmp %s double %%r%d, %%r%d\n", r, op, lv, rv);
        } else if (use_bool) {
            const char *op = (strcmp(n->kind,"Eq")==0) ? "eq" : "ne";
            printf("  %%r%d = icmp %s i1 %%r%d, %%r%d\n", r, op, lv, rv);
        } else {
            const char *op = strcmp(n->kind,"Eq")==0 ? "eq"  :
                             strcmp(n->kind,"Ne")==0 ? "ne"  :
                             strcmp(n->kind,"Lt")==0 ? "slt" :
                             strcmp(n->kind,"Gt")==0 ? "sgt" :
                             strcmp(n->kind,"Le")==0 ? "sle" : "sge";
            printf("  %%r%d = icmp %s i32 %%r%d, %%r%d\n", r, op, lv, rv);
        }
        return r;
    }

    // --- Assign (atribuição) ---
    if (strcmp(n->kind, "Assign") == 0) {
        Node *lhsn = ch(n, 0); // Identifier (LHS)
        Node *rhsn = ch(n, 1);
        int rv = cg_expr(rhsn, meth);
        const char *lhs_t = lhsn->type ? lhsn->type : "int";
        int sv = widen(rv, rhsn->type, lhs_t);
        emit_store(lhsn->val, lhs_t, sv, meth);
        return sv;
    }

    // --- Length (array.length) ---
    if (strcmp(n->kind, "Length") == 0) {
        Node *arr = ch(n, 0);
        int r = g_tmp++;
        // argc contém o número real de argumentos + 1 (nome do programa)
        printf("  %%r%d = sub i32 %%argc_%s, 1\n", r, arr->val);
        return r;
    }

    // --- ParseArgs (Integer.parseInt(arr[idx])) ---
    if (strcmp(n->kind, "ParseArgs") == 0) {
        Node *arr = ch(n, 0);
        Node *idx = ch(n, 1);
        int idx_r = cg_expr(idx, meth);
        // argv[0] é o nome do programa; args[i] = argv[i+1]
        int adj = g_tmp++;
        printf("  %%r%d = add i32 %%r%d, 1\n", adj, idx_r);
        int ptr = g_tmp++;
        printf("  %%r%d = getelementptr i8*, i8** %%argv_%s, i32 %%r%d\n",
               ptr, arr->val, adj);
        int str = g_tmp++;
        printf("  %%r%d = load i8*, i8** %%r%d\n", str, ptr);
        int res = g_tmp++;
        printf("  %%r%d = call i32 @atoi(i8* %%r%d)\n", res, str);
        return res;
    }

    // --- Call (chamada de método) ---
    if (strcmp(n->kind, "Call") == 0) {
        Node *id = ch(n, 0);
        const char *mname = id->val;
        const char *psig  = id->type; // param_sig anotado pela análise semântica

        ClassSym *ms = find_msym(mname, psig);

        // Nome mangled do método
        char fname[256];
        mangle(fname, sizeof(fname), mname, psig);

        // Construir lista de argumentos LLVM
        char argbuf[8192]; argbuf[0] = '\0';
        int nargs = n->nchildren - 1;
        int param_idx = 0;

        for (int i = 0; i < nargs; i++) {
            Node *arg = ch(n, i + 1);
            const char *param_t = (ms && param_idx < ms->num_params)
                                  ? ms->param_types[param_idx] : "int";

            if (argbuf[0]) strcat(argbuf, ", ");

            if (strcmp(param_t, "String[]") == 0) {
                // Parâmetro String[]: passa argc e argv diretamente
                const char *arr_name = arg->val;
                char tmp[256];
                snprintf(tmp, sizeof(tmp), "i32 %%argc_%s, i8** %%argv_%s",
                         arr_name, arr_name);
                strcat(argbuf, tmp);
            } else {
                int av = cg_expr(arg, meth);
                av = widen(av, arg->type, param_t);
                char tmp[128];
                snprintf(tmp, sizeof(tmp), "%s %%r%d", ll_type(param_t), av);
                strcat(argbuf, tmp);
            }
            param_idx++;
        }

        const char *ret_t = ms ? ms->type : "void";
        if (strcmp(ret_t, "void") == 0) {
            printf("  call void @%s(%s)\n", fname, argbuf);
            return 0;
        } else {
            int r = g_tmp++;
            printf("  %%r%d = call %s @%s(%s)\n", r, ll_type(ret_t), fname, argbuf);
            return r;
        }
    }

    return 0;
}

// ====================================================================
// Geração de código para instruções
// ====================================================================
static void cg_stmt(Node *n, ClassSym *meth) {
    if (!n) return;

    // VarDecl: ignorar – a alloca foi emitida na entrada da função
    if (strcmp(n->kind, "VarDecl") == 0) return;

    // Block
    if (strcmp(n->kind, "Block") == 0) {
        for (int i = 0; i < n->nchildren; i++)
            cg_stmt(n->children[i], meth);
        return;
    }

    // If
    if (strcmp(n->kind, "If") == 0) {
        int lbl = g_lbl++;
        int cr = cg_expr(ch(n, 0), meth);
        if (!g_term)
            printf("  br i1 %%r%d, label %%then%d, label %%else%d\n", cr, lbl, lbl);
        g_term = 0;
        // Ramo then
        printf("then%d:\n", lbl);
        snprintf(g_cur_blk, sizeof(g_cur_blk), "then%d", lbl);
        cg_stmt(ch(n, 1), meth);
        if (!g_term) printf("  br label %%end%d\n", lbl);
        g_term = 0;
        // Ramo else
        printf("else%d:\n", lbl);
        snprintf(g_cur_blk, sizeof(g_cur_blk), "else%d", lbl);
        cg_stmt(ch(n, 2), meth);
        if (!g_term) printf("  br label %%end%d\n", lbl);
        g_term = 0;
        // Continuação
        printf("end%d:\n", lbl);
        snprintf(g_cur_blk, sizeof(g_cur_blk), "end%d", lbl);
        return;
    }

    // While
    if (strcmp(n->kind, "While") == 0) {
        int lbl = g_lbl++;
        if (!g_term) printf("  br label %%cond%d\n", lbl);
        g_term = 0;
        // Bloco de condição
        printf("cond%d:\n", lbl);
        snprintf(g_cur_blk, sizeof(g_cur_blk), "cond%d", lbl);
        int cr = cg_expr(ch(n, 0), meth);
        if (!g_term)
            printf("  br i1 %%r%d, label %%body%d, label %%wend%d\n", cr, lbl, lbl);
        g_term = 0;
        // Corpo do ciclo
        printf("body%d:\n", lbl);
        snprintf(g_cur_blk, sizeof(g_cur_blk), "body%d", lbl);
        cg_stmt(ch(n, 1), meth);
        if (!g_term) printf("  br label %%cond%d\n", lbl);
        g_term = 0;
        // Fim do ciclo
        printf("wend%d:\n", lbl);
        snprintf(g_cur_blk, sizeof(g_cur_blk), "wend%d", lbl);
        return;
    }

    // Return
    if (strcmp(n->kind, "Return") == 0) {
        LocalSym *rs = meth ? loc_find(meth->locals, "return") : NULL;
        const char *ret_t = rs ? rs->type : "void";
        Node *expr = ch(n, 0);
        if (!expr) {
            printf("  ret void\n");
        } else {
            int rv = cg_expr(expr, meth);
            rv = widen(rv, expr->type, ret_t);
            printf("  ret %s %%r%d\n", ll_type(ret_t), rv);
        }
        g_term = 1;
        return;
    }

    // Print (System.out.print)
    if (strcmp(n->kind, "Print") == 0) {
        Node *arg = ch(n, 0);
        if (!arg) return;
        if (strcmp(arg->kind, "StrLit") == 0) {
            // String literal: usar constante global
            int idx = find_str(arg->val);
            int sp  = emit_strptr(idx);
            int fmt = emit_fmtptr("fmt_str", 3);
            printf("  call i32 (i8*, ...) @printf(i8* %%r%d, i8* %%r%d)\n", fmt, sp);
        } else {
            int vr = cg_expr(arg, meth);
            emit_print_val(vr, arg->type ? arg->type : "int");
        }
        return;
    }

    // Assign / Call / ParseArgs usados como instrução (ignorar resultado)
    cg_expr(n, meth);
}

// ====================================================================
// Geração de código para um método
// ====================================================================
static void cg_method(ClassSym *ms, Node *mdecl) {
    g_tmp  = 1;
    g_term = 0;
    snprintf(g_cur_blk, sizeof(g_cur_blk), "entry");

    // Cabeçalho: define TYPE @_nome_params(params) {
    printf("define %s @", ll_type(ms->type));
    char fname[256];
    mangle(fname, sizeof(fname), ms->name, ms->param_sig);
    printf("%s(", fname);

    int first = 1;
    for (LocalSym *ls = ms->locals; ls; ls = ls->next) {
        if (!ls->is_param) continue;
        if (!first) printf(", ");
        first = 0;
        if (strcmp(ls->type, "String[]") == 0)
            printf("i32 %%argc_%s, i8** %%argv_%s", ls->name, ls->name);
        else
            printf("%s %%param_%s", ll_type(ls->type), ls->name);
    }
    printf(") {\nentry:\n");

    // Alloca para variáveis locais e parâmetros (exceto String[])
    for (LocalSym *ls = ms->locals; ls; ls = ls->next) {
        if (strcmp(ls->name, "return") == 0) continue;
        if (strcmp(ls->type, "String[]") == 0) continue;
        printf("  %%local_%s = alloca %s\n", ls->name, ll_type(ls->type));
    }

    // Store dos valores dos parâmetros nas suas allocas
    for (LocalSym *ls = ms->locals; ls; ls = ls->next) {
        if (!ls->is_param) continue;
        if (strcmp(ls->type, "String[]") == 0) continue;
        printf("  store %s %%param_%s, %s* %%local_%s\n",
               ll_type(ls->type), ls->name, ll_type(ls->type), ls->name);
    }

    // Gerar corpo do método (filhos do MethodBody)
    Node *body = ch(mdecl, 1);
    if (body) {
        for (int i = 0; i < body->nchildren; i++) {
            Node *stmt = body->children[i];
            if (stmt) cg_stmt(stmt, ms);
        }
    }

    // Retorno implícito se o bloco não foi terminado
    if (!g_term) {
        if (strcmp(ms->type, "void") == 0)
            printf("  ret void\n");
        else if (strcmp(ms->type, "double") == 0)
            printf("  ret double 0.0\n");
        else
            printf("  ret %s 0\n", ll_type(ms->type));
    }

    printf("}\n\n");
}

// ====================================================================
// Ponto de entrada: gera LLVM IR para o programa completo
// ====================================================================
void codegen_program(Node *root) {
    if (!root) return;

    // Pré-varredura de strings literais
    g_nstrs = 0;
    scan_strs(root);
    g_lbl = 0;

    // --- Strings de formato ---
    printf("@.fmt_int    = private unnamed_addr constant [3 x i8] c\"%%d\\00\"\n");
    printf("@.fmt_double = private unnamed_addr constant [6 x i8] c\"%%.16e\\00\"\n");
    printf("@.fmt_str    = private unnamed_addr constant [3 x i8] c\"%%s\\00\"\n");
    printf("@.str_true   = private unnamed_addr constant [5 x i8] c\"true\\00\"\n");
    printf("@.str_false  = private unnamed_addr constant [6 x i8] c\"false\\00\"\n");

    // --- Strings literais do programa ---
    for (int i = 0; i < g_nstrs; i++) {
        char hbuf[16384];
        str_to_llvm(g_str_val[i], hbuf, sizeof(hbuf));
        printf("@.str%d = private unnamed_addr constant [%d x i8] c\"%s\"\n",
               i, g_str_blen[i] + 1, hbuf);
    }
    printf("\n");

    // --- Declarações externas ---
    printf("declare i32 @printf(i8*, ...)\n");
    printf("declare i32 @atoi(i8*)\n");
    printf("\n");

    // --- Campos globais (fields) ---
    int has_fields = 0;
    for (ClassSym *s = g_class_table; s; s = s->next) {
        if (!s->is_method) {
            printf("@%s = global %s %s\n",
                   s->name, ll_type(s->type), ll_zero(s->type));
            has_fields = 1;
        }
    }
    if (has_fields) printf("\n");

    // --- Métodos: iterar filhos do Program por ordem de declaração ---
    for (int i = 1; i < root->nchildren; i++) {
        Node *member = root->children[i];
        if (!member || strcmp(member->kind, "MethodDecl") != 0) continue;

        // Obter nome + param_sig para localizar o ClassSym
        Node *hdr    = ch(member, 0); // MethodHeader
        Node *namend = ch(hdr, 1);    // Identifier
        Node *parmsnd = ch(hdr, 2);   // MethodParams
        if (!namend) continue;

        const char *mname = namend->val;
        int nparams = parmsnd ? parmsnd->nchildren : 0;
        char **ptypes = nparams ? malloc(nparams * sizeof(char*)) : NULL;
        for (int j = 0; j < nparams; j++) {
            Node *pdecl = ch(parmsnd, j);
            Node *ptype_node = ch(pdecl, 0);
            ptypes[j] = (char *)kind2type(ptype_node ? ptype_node->kind : NULL);
        }
        char *psig = build_psig(ptypes, nparams);
        free(ptypes);

        ClassSym *ms = find_msym(mname, psig);
        free(psig);

        if (ms) cg_method(ms, member);
    }

    //  Ponto de entrada @main
    ClassSym *main_ms = find_msym("main", "(String[])");
    if (main_ms) {
        char fname[256];
        mangle(fname, sizeof(fname), "main", "(String[])");
        printf("define i32 @main(i32 %%argc, i8** %%argv) {\n");
        printf("entry:\n");
        printf("  call void @%s(i32 %%argc, i8** %%argv)\n", fname);
        printf("  ret i32 0\n");
        printf("}\n");
    }
}
