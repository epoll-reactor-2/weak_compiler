//CompoundStmt <line:0, col:0>
//  FnDecl <line:14, col:1>
//    FnDeclRetType <line:14, col:1> void
//    FnDeclName <line:14, col:1> `f`
//    FnDeclArgs <line:14, col:1>
//    FnDeclBody <line:14, col:1>
//      CompoundStmt <line:14, col:10>
//        VarDecl <line:15, col:3> float `value`
//          FloatLiteral <line:15, col:17> 3.141500
//        VarDecl <line:16, col:3> boolean `good_boolean`
//          BooleanLiteral <line:16, col:23> true
//        VarDecl <line:17, col:3> boolean `malevolent_boolean`
//          BooleanLiteral <line:17, col:29> false
void f() {
  float value = 3.1415;
  bool good_boolean = true;
  bool malevolent_boolean = false;
}
