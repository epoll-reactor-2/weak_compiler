//CompoundStmt <line:0, col:0>
//  FnDecl <line:19, col:1>
//    FnDeclRetType <line:19, col:1> int
//    FnDeclName <line:19, col:1> `main`
//    FnDeclArgs <line:19, col:1>
//    FnDeclBody <line:19, col:1>
//      CompoundStmt <line:19, col:12>
//        FnDecl <line:20, col:5>
//          FnDeclRetType <line:20, col:5> int
//          FnDeclName <line:20, col:5> `return_0`
//          FnDeclArgs <line:20, col:5>
//          FnDeclBody <line:20, col:5>
//            CompoundStmt <line:20, col:20>
//              ReturnStmt <line:21, col:9>
//                Number <line:21, col:16> 0
//        ReturnStmt <line:23, col:5>
//          FnCall <line:23, col:12> `return_0`
//            FnCallArgs <line:23, col:12>
int main() {
    int return_0() {
        return 0;
    }
    return return_0();
}