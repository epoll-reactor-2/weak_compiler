//CompoundStmt <line:0, col:0>
//  FnDecl <line:26, col:1>
//    FnDeclRetType <line:26, col:1> int
//    FnDeclName <line:26, col:1> `f`
//    FnDeclArgs <line:26, col:1>
//    FnDeclBody <line:26, col:1>
//      CompoundStmt <line:26, col:9>
//        ReturnStmt <line:27, col:3>
//          BinaryOperator <line:27, col:12> +
//            Symbol <line:27, col:10> `a`
//            FnCall <line:27, col:14> `b`
//              FnCallArgs <line:27, col:14>
//                CompoundStmt <line:27, col:14>
//                  FnCall <line:27, col:16> `c`
//                    FnCallArgs <line:27, col:16>
//                      CompoundStmt <line:27, col:16>
//                        BinaryOperator <line:27, col:20> +
//                          Number <line:27, col:18> 1
//                          BinaryOperator <line:27, col:27> +
//                            FnCall <line:27, col:22> `d`
//                              FnCallArgs <line:27, col:22>
//                                CompoundStmt <line:27, col:22>
//                                  Symbol <line:27, col:24> `e`
//                            Number <line:27, col:29> 1
//                  Symbol <line:27, col:33> `f`
int f() {
  return a + b(c(1 + d(e) + 1), f);
}
