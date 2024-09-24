//CompoundStmt <line:0, col:0>
//  StructDecl <line:16, col:1> `custom`
//    CompoundStmt <line:16, col:1>
//      VarDecl <line:17, col:5> int `a`
//      VarDecl <line:18, col:5> int `b`
//      VarDecl <line:19, col:5> int `c`
//      ArrayDecl <line:20, col:5> char [1000] `mem`
//      VarDecl <line:21, col:5> struct string `description`
//  FnDecl <line:24, col:1>
//    FnDeclRetType <line:24, col:1> void
//    FnDeclName <line:24, col:1> `f`
//    FnDeclArgs <line:24, col:1>
//    FnDeclBody <line:24, col:1>
//      CompoundStmt <line:24, col:10>
//        VarDecl <line:25, col:5> struct custom `x`
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
