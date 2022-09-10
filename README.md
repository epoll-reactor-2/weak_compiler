[![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://epoll-reactor.github.io/weak_compiler/index.html)

# Impregnation
This is an implementation of simple (or not so simple, XD) compiler,
which uses LLVM.

# Conception

## About

This project designed to be modular, easy-embeddedable to other projects
(like it was done [there](https://github.com/epoll-reactor/algorithm_bot/blob/master/bot/src/modules/compiler.cpp)).

There is a front-end (building of AST), which can be connected to many back-ends,
such as LLVM (which is already implemented), and other code generators, including
self-written ones.

## Language

This is a classical example of C-like programming language with static strong
type system. It means, any implicit conversions are not allowed, and we should
use cast functions like `static_cast` in C++. However, this is not implemented
because of the undecided way of embedding built-in functions to language.

## Interface & capabilities

Let me show how we can use it from command line:
```
$ cat example.wl
// Declared in libc.
int puts(string arg);

int main() {
	string input = "Hello, World!";
	return puts(input);
}
```
If we're want to see tokens:
```
$ ./Compiler -i example.wl -dump-lexemes
Token                <INT>  
Token             <SYMBOL>  puts
Token                    (  
Token             <STRING>  
Token             <SYMBOL>  arg
Token                    )  
Token                    ;  
Token                <INT>  
Token             <SYMBOL>  main
Token                    (  
Token                    )  
Token                    {  
Token             <STRING>  
Token             <SYMBOL>  input
Token                    =  
Token     <STRING_LITERAL>  Hello, World!
Token                    ;  
Token             <RETURN>  
Token             <SYMBOL>  puts
Token                    (  
Token             <SYMBOL>  input
Token                    )  
Token                    ;  
Token                    }
```
If we're want to see syntax tree:
```
$ ./Compiler -i example.wl -dump-ast
CompoundStmt <line:0, col:0>
  FunctionPrototype <line:1, col:1> puts
    FunctionPrototypeArgs <line:1, col:1>
      VarDeclStmt <line:1, col:10> <STRING> arg
  FunctionDecl <line:3, col:1>
    FunctionDeclRetType <line:3, col:1> <INT>
    FunctionDeclName <line:3, col:1> main
    FunctionDeclArgs <line:3, col:1>
    FunctionDeclBody <line:3, col:1>
      CompoundStmt <line:3, col:12>
        VarDeclStmt <line:4, col:2> <STRING> input
          StringLiteral <line:4, col:17> Hello, World!
        ReturnStmt <line:5, col:2>
          FunctionCall <line:5, col:9> puts
            FunctionCallArgs <line:5, col:9>
              Symbol <line:5, col:14> input
```
If we're want to see LLVM IR:
```
$ ./Compiler -i example.wl -dump-llvm
@0 = constant [14 x i8] c"Hello, World!\00"

declare i32 @puts(i8*)

define i32 @main() {
entry:
  %input = alloca i8*, align 8
  store i8* getelementptr inbounds ([14 x i8], [14 x i8]* @0, i32 0, i32 0), i8** %input, align 8
  %input1 = load i8*, i8** %input, align 8
  %0 = call i32 @puts(i8* %input1)
  ret i32 %0
}
```
And if we're want just to get executable file:
```
$ ./Compiler -i example.wl -o example
$ ./example
Hello, World!
```

# Termination

## TODO
* Self-written back-end
  * IR
  * register allocation
  * linker (and way to combine many source files to one executable)
* optimizations
  * graph-based
    * SSA (implemented in master)
  * instructions combining
  * and others...
* something similar to standard library (libc wrappers)
* well-defined type system
* scopes
* handling of multiple definitions
* clear API to be able to develop freestanding utilities (such as code formatters, static analyzers)
