//CompoundStmt <line:0, col:0>
//  FnDecl <line:16, col:1>
//    FnDeclRetType <line:16, col:1> int
//    FnDeclName <line:16, col:1> `f`
//    FnDeclArgs <line:16, col:1>
//    FnDeclBody <line:16, col:1>
//      CompoundStmt <line:16, col:9>
//        ReturnStmt <line:17, col:3>
//          BinaryOperator <line:17, col:12> +
//            Symbol <line:17, col:10> `x`
//            FnCall <line:17, col:14> `y`
//              FnCallArgs <line:17, col:14>
//                CompoundStmt <line:17, col:14>
//                  Symbol <line:17, col:16> `z`
//                  Symbol <line:17, col:19> `q`
int f() {
  return x + y(z, q);
}
