//CompoundStmt <line:0, col:0>
//  FnDecl <line:15, col:1>
//    FnDeclRetType <line:15, col:1> int
//    FnDeclName <line:15, col:1> `f`
//    FnDeclArgs <line:15, col:1>
//    FnDeclBody <line:15, col:1>
//      CompoundStmt <line:15, col:9>
//        ReturnStmt <line:16, col:3>
//          FnCall <line:16, col:10> `call`
//            FnCallArgs <line:16, col:10>
//              CompoundStmt <line:16, col:10>
//                Number <line:16, col:15> 1
//                Number <line:16, col:18> 2
//                Number <line:16, col:21> 3
int f() {
  return call(1, 2, 3);
}
