#!/usr/bin/env bash

set -e

echo "==> bison -dy jucompiler.y"
bison -dy jucompiler.y

echo "==> flex jucompiler.l"
flex jucompiler.l

echo "==> gcc y.tab.c lex.yy.c ast.c semantics.c codegen.c -o jucompiler"
gcc y.tab.c lex.yy.c ast.c semantics.c codegen.c -o jucompiler

if [[ -n "$1" && -f "$1" ]]; then
    input="$1"
    out="${input%.*}.out"
    echo "==> ./jucompiler -s < $input > $out"
    ./jucompiler -s < "$input" > "$out"
    echo "==> Output guardado em $out"
fi

