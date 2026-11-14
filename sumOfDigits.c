#include <stdio.h>

int extractDigitAndSum(int n)
{
    int sum = 0;
    while(n>0)
    {
        int digit = n%10;
        sum += digit;
        n = n/10;
    }
    return sum;
}

int main() {
	
    int n;
    scanf("%d", &n);
    printf("%d", extractDigitAndSum(n));
    //Complete the code to calculate the sum of the five digits on n.
    return 0;
}
