//CompoundStmt <line:0, col:0>
//  FnDecl <line:11, col:1>
//    FnDeclRetType <line:11, col:1> int
//    FnDeclName <line:11, col:1> `f`
//    FnDeclArgs <line:11, col:1>
//    FnDeclBody <line:11, col:1>
//      CompoundStmt <line:11, col:9>
//        ReturnStmt <line:12, col:3>
//          FnCall <line:12, col:10> `call`
//            FnCallArgs <line:12, col:10>
int f() {
  return call();
}