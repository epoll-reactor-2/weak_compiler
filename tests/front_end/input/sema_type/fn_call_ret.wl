//CompoundStmt
//  FnDecl
//    FnDeclRetType float
//    FnDeclName `f`
//    FnDeclArgs
//      CompoundStmt
//        VarDecl int `arg`
//    FnDeclBody
//      CompoundStmt
//        ReturnStmt
//          ImplicitCastExpr -> float
//            Number 0
//  FnDecl
//    FnDeclRetType int
//    FnDeclName `main`
//    FnDeclArgs
//    FnDeclBody
//      CompoundStmt
//        ReturnStmt
//          ImplicitCastExpr -> int
//            FnCall `f`
//              FnCallArgs
//                CompoundStmt
//                  ImplicitCastExpr -> int
//                    FloatLiteral 0.000000
float f(int arg) {
    return 0;
}

int main() {
    return f(0.0);
}