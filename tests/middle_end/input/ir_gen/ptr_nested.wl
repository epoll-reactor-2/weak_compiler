//a
int main() {
    int     a = 1;
    int    *b = &a;
    int   **c = &b;
    int  ***d = &c;
    int ****e = &d;

    return ****e;
}