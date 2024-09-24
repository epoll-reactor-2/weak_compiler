//CompoundStmt <line:0, col:0>
//  FnDecl <line:14, col:1>
//    FnDeclRetType <line:14, col:1> int
//    FnDeclName <line:14, col:1> `f`
//    FnDeclArgs <line:14, col:1>
//    FnDeclBody <line:14, col:1>
//      CompoundStmt <line:14, col:9>
//        ReturnStmt <line:15, col:5>
//          BinaryOperator <line:15, col:16> ==
//            FnCall <line:15, col:12> `x`
//              FnCallArgs <line:15, col:12>
//            FnCall <line:15, col:19> `y`
//              FnCallArgs <line:15, col:19>
int f() {
    return x() == y();
}