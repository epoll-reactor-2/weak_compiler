//CompoundStmt <line:0, col:0>
//  FnProto <line:32, col:1>
//    FnProtoRetType <line:32, col:1> void
//    FnProtoName <line:32, col:1> `f`
//    FnProtoArgs <line:32, col:1>
//      CompoundStmt <line:32, col:16>
//        VarDecl <line:32, col:8> int `size`
//  FnProto <line:33, col:1>
//    FnProtoRetType <line:33, col:1> void
//    FnProtoName <line:33, col:1> `f`
//    FnProtoArgs <line:33, col:1>
//      CompoundStmt <line:33, col:38>
//        ArrayDecl <line:33, col:8> int [200000000] `array`
//        VarDecl <line:33, col:30> int `size`
//  FnProto <line:34, col:1>
//    FnProtoRetType <line:34, col:1> void
//    FnProtoName <line:34, col:1> `f`
//    FnProtoArgs <line:34, col:1>
//      CompoundStmt <line:34, col:38>
//        VarDecl <line:34, col:8> int `size`
//        ArrayDecl <line:34, col:18> int [300000000] `array`
//  FnDecl <line:36, col:1>
//    FnDeclRetType <line:36, col:1> int
//    FnDeclName <line:36, col:1> `main`
//    FnDeclArgs <line:36, col:1>
//    FnDeclBody <line:36, col:1>
//      CompoundStmt <line:36, col:12>
//        VarDecl <line:37, col:5> int `variable`
//          Number <line:37, col:20> 0
//        ArrayDecl <line:38, col:5> int [3] `array`
//        ArrayDecl <line:39, col:5> int [1][2][3] `array`
void f(int size);
void f(int array[200000000], int size);
void f(int size, int array[300000000]);

int main() {
    int variable = 0;
    int array[3];
    int array[1][2][3];
}
