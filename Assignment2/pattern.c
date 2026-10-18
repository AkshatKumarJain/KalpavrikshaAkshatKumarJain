#include <stdio.h>

void printFigure(int n)
{
        for(int i=0;i<2*n-1;i++) {
            for(int j=0; j<2*n-1;j++) {
                int top = i;
                int left = j;
                int bottom = 2*n-2-i;
                int right = 2*n-2-j;
                int min = top;
                if(left<min)
                {
                    min = left;
                }
                if(bottom<min)
                {
                    min = bottom;
                }
                if(right<min)
                {
                    min = right;
                }

            printf("%d ", n-min);
        }
        printf("\n");
    }
}

int main() 
{

    int n;
    scanf("%d", &n);
  	// Complete the code to print the pattern.
    printFigure(n);
    return 0;
}