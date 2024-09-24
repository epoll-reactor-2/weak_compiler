//CompoundStmt <line:0, col:0>
//  FnDecl <line:15, col:1>
//    FnDeclRetType <line:15, col:1> void
//    FnDeclName <line:15, col:1> `f`
//    FnDeclArgs <line:15, col:1>
//    FnDeclBody <line:15, col:1>
//      CompoundStmt <line:15, col:10>
//        ForRangeStmt <line:16, col:3>
//          ForRangeIterStmt <line:16, col:8>
//            VarDecl <line:16, col:8> int `i`
//          ForRangeTargetStmt <line:16, col:16>
//            Symbol <line:16, col:16> `array`
//          ForRangeStmtBody <line:16, col:23>
//            CompoundStmt <line:16, col:23>
void f() {
  for (int i : array) {}
}
