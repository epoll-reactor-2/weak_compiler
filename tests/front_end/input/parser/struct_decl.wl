//CompoundStmt <line:0, col:0>
//  StructDecl <line:9, col:1> `custom`
//    CompoundStmt <line:9, col:1>
//      VarDecl <line:10, col:5> int `a`
//      VarDecl <line:11, col:5> int `b`
//      VarDecl <line:12, col:5> int `c`
//      ArrayDecl <line:13, col:5> char [1000] `mem`
//      VarDecl <line:14, col:5> struct string `description`
struct custom {
    int a;
    int b;
    int c;
    char mem[1000];
    string description;
}
