#include <stdio.h>

/***
 * 本程序使用欧几里得算法计算两个数的最大公约数
*/

int gcd(int m, int n);
int main()
{
    printf("%d\n",gcd(60,24));
    return 0;
}

int gcd(int m, int n)
{
    if(!n)
        return m;
    return gcd(n, m%n);
}