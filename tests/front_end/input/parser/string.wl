//CompoundStmt <line:0, col:0>
//  FnDecl <line:12, col:1>
//    FnDeclRetType <line:12, col:1> int
//    FnDeclName <line:12, col:1> `main`
//    FnDeclArgs <line:12, col:1>
//    FnDeclBody <line:12, col:1>
//      CompoundStmt <line:12, col:12>
//        VarDecl <line:13, col:5> char * `s`
//          StringLiteral <line:13, col:15> Abc
//        ReturnStmt <line:14, col:5>
//          Number <line:14, col:12> 0
int main() {
    char *s = "Abc";
    return 0;
}