#include<stdio.h>
#include<ctype.h>

int main() 
{
    char expression[1000];
    long long i = 0;
    long long res = 0;
    long long curr = 0;
    long long num = 0;
    char op = '+';
    int hasNumber = 0;
    fgets(expression, sizeof(expression), stdin);
    while(expression[i]!='\0' && expression[i]!='\n')
    {
        if(expression[i]==' ' || expression[i]=='\t')
        {
            i++;
            continue;
        }
        if(!isdigit(expression[i]))
        {
            printf("Error: Invalid Expression!");
            return 0;
        }
        num = 0;
        while(isdigit(expression[i]))
        {
            num = num*10+(expression[i]-'0');
            i++;
        }
        hasNumber = 1;
        if(op=='+')
        {
            res += curr;
            curr = num;
        }
        else if(op=='-')
        {
            res += curr;
            curr = -num;
        }
        else if(op=='*')
        {
            curr *= num;
        }
        else if(op=='/')
        {
            if(num==0)
            {
                printf("Error: Division By Zero");
                return 0;
            }
            curr /= num;
        }
    
    while(expression[i]==' ' || expression[i]=='\t')
    {
        i++;
    }
    if(expression[i]=='+' || expression[i]=='-' || expression[i]=='*' || expression[i]=='/')
    {
        op = expression[i];
        i++;
        hasNumber = 0;
    }
    else if(expression[i]!='\0' && expression[i]!='\n')
    {
        printf("Error: Invalid Expression");
        return 0;
    }
}
    if(!hasNumber)
    {
        printf("Error: Invalid Expression!");
        return 0;
    }
    res += curr;
    printf("%lld\n", res);
}