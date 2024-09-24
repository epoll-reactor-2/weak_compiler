//CompoundStmt
//  FnDecl
//    FnDeclRetType boolean
//    FnDeclName `g`
//    FnDeclArgs
//    FnDeclBody
//      CompoundStmt
//        ReturnStmt
//          ImplicitCastExpr -> boolean
//            Number 1
bool g() {
    return 1;
}
