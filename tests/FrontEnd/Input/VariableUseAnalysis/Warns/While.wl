// Warning at line 5, column 5: Variable `j` written, but never read
// Warning at line 7, column 5: Variable `l` is never used
int main() {
    int i = 0;
    int j = 0;
    int k = 0;
    int l = 0;
    while (i) {
        while (i) {
            while (i) {
                while (k) {
                    ++i;
                    ++j;
                }
            }
        }
    }
    return 0;
}
