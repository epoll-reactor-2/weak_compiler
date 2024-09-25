//a
int sum_2()
{
    int r = 1 + 2;
    return r;
}

int sum_3()
{
    sum_2();
    int r = 1 + 2 + 3;
    return r;
}

int main()
{
    sum_3();
    return 0;
}