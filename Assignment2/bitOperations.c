#include <stdio.h>
//Complete the following function.


void calculate_the_maximum(int n, int k) {
  //Write your code here.
  int maxA = 0;
  int maxO = 0;
  int maxX = 0;
  for(int i=1;i<=n;i++)
  {
    for(int j=i+1;j<=n;j++)
    {
        int and = i&j;
        int or = i|j;
        int xor = i^j;
        if(and<k && and>maxA)
        {
            maxA = and;
        }
        if(or<k && or>maxO)
        {
            maxO = or;
        }
        if(xor<k && xor>maxX)
        {
            maxX = xor;
        }
    }
  }
  printf("%d\n", maxA);
  printf("%d\n", maxO);
  printf("%d\n", maxX);
}

int main() {
    int n, k;
  
    scanf("%d %d", &n, &k);
    calculate_the_maximum(n, k);
 
    return 0;
}
