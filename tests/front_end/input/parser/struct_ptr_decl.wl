//CompoundStmt <line:0, col:0>
//  FnDecl <line:21, col:1>
//    FnDeclRetType <line:21, col:1> int
//    FnDeclName <line:21, col:1> `main`
//    FnDeclArgs <line:21, col:1>
//    FnDeclBody <line:21, col:1>
//      CompoundStmt <line:21, col:12>
//        VarDecl <line:22, col:5> struct x * `y`
//        VarDecl <line:23, col:5> struct x ** `z`
//        VarDecl <line:24, col:5> struct a * `f`
//        VarDecl <line:25, col:5> int `first`
//          BinaryOperator <line:25, col:21> *
//            FnCall <line:25, col:17> `f`
//              FnCallArgs <line:25, col:17>
//            Symbol <line:25, col:23> `a`
//        VarDecl <line:26, col:5> int `second`
//          BinaryOperator <line:26, col:20> *
//            Symbol <line:26, col:18> `a`
//            FnCall <line:26, col:22> `f`
//              FnCallArgs <line:26, col:22>
int main() {
    x *y;
    x **z;
    a *f;
    int first = f() * a;
    int second = a * f();
}
