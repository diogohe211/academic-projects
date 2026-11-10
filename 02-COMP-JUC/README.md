# JUC Compiler

Compiler for a Java-like teaching language. Flex and Bison implement lexical and syntax analysis, followed by an abstract syntax tree, semantic checks and LLVM IR generation.

## Requirements

Linux or another environment with Bash, GCC, Flex and Bison.

## Run

```sh
bash build.sh
./jucompiler < input.java > output.ll
```

Replace input.java with a program in the supported teaching language. Use -s for semantic-analysis output. The optional argument to build.sh runs the compiler with -s.

## Project notes

This is the academic language subset implemented in jucompiler.l and jucompiler.y. LLVM tools are needed to process the generated IR; compatible LLVM versions require verification.
