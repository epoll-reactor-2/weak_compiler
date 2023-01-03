//CompoundStmt <line:0, col:0>
//  StructDecl <line:15, col:7> `custom`
//    Indexed VarDecl <line:16, col:5> int `a` `0`
//    Indexed VarDecl <line:17, col:5> int `b` `1`
//    Indexed VarDecl <line:18, col:5> int `c` `2`
//    Indexed ArrayDecl <line:19, col:5> char [1000] `mem` `3`
//    Indexed VarDecl <line:20, col:5> string `description` `4`
//  FunctionDecl <line:23, col:1>
//    FunctionDeclRetType <line:23, col:1> void
//    FunctionDeclName <line:23, col:1> `f`
//    FunctionDeclArgs <line:23, col:1>
//    FunctionDeclBody <line:23, col:1>
//      CompoundStmt <line:23, col:10>
//        VarDecl <line:24, col:5> struct custom `x`
struct custom {
    int a;
    int b;
    int c;
    char mem[1000];
    string description;
}

void f() {
    custom x;
}
