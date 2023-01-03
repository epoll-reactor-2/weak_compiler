//CompoundStmt <line:0, col:0>
//  StructDecl <line:8, col:7> `custom`
//    Indexed VarDecl <line:9, col:5> int `a` `0`
//    Indexed VarDecl <line:10, col:5> int `b` `1`
//    Indexed VarDecl <line:11, col:5> int `c` `2`
//    Indexed ArrayDecl <line:12, col:5> char [1000] `mem` `3`
//    Indexed VarDecl <line:13, col:5> string `description` `4`
struct custom {
    int a;
    int b;
    int c;
    char mem[1000];
    string description;
}
