//CompoundStmt <line:0, col:0>
//  StructDecl <line:29, col:7> `custom`
//    Indexed VarDecl <line:30, col:5> int `a` `0`
//    Indexed VarDecl <line:31, col:5> int `b` `1`
//    Indexed VarDecl <line:32, col:5> int `c` `2`
//    StructDecl <line:33, col:11> `nested`
//      Indexed VarDecl <line:34, col:9> int `d` `0`
//      Indexed VarDecl <line:35, col:9> int `e` `1`
//      StructDecl <line:36, col:15> `nested_too_much`
//        Indexed VarDecl <line:37, col:13> int `f` `0`
//        Indexed VarDecl <line:38, col:13> int `g` `1`
//        Indexed VarDecl <line:39, col:13> int `h` `2`
//  FunctionDecl <line:44, col:1>
//    FunctionDeclRetType <line:44, col:1> void
//    FunctionDeclName <line:44, col:1> `f`
//    FunctionDeclArgs <line:44, col:1>
//    FunctionDeclBody <line:44, col:1>
//      CompoundStmt <line:44, col:10>
//        VarDecl <line:45, col:5> struct custom `x`
//        BinaryOperator <line:46, col:32> =
//          StructMemberAccess <line:46, col:5>
//            Symbol <line:46, col:5> `x`
//            StructMemberAccess <line:46, col:7>
//              Symbol <line:46, col:7> `nested`
//              StructMemberAccess <line:46, col:14>
//                Symbol <line:46, col:14> `nested_too_much`
//                Symbol <line:46, col:30> `f`
//          Number <line:46, col:34> 1
struct custom {
    int a;
    int b;
    int c;
    struct nested {
        int d;
        int e;
        struct nested_too_much {
            int f;
            int g;
            int h;
        };
    };
}

void f() {
    custom x;
    x.nested.nested_too_much.f = 1;
}
