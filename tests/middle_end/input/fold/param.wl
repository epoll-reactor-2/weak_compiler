//fun f(alloca int %0, alloca int %1):
//       2:   alloca int %2
//       3:   store %2 $10
//       4:   alloca int %3
//       5:   alloca int %4
//       6:   alloca int %5
//       7:   alloca int %6
//       8:   store %6 $12
//       9:   store %5 %0 mul $12
//      10:   store %4 %5 add %1
//      11:   store %3 %4 shl $10
//      12:   ret %3
int f(int a, int b)
{
    int const = 10;

    return a * (const + 2) + b << const;
}