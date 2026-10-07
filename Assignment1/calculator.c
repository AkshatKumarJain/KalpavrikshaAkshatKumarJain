#include<stdio.h>

// define status code for error handling
#define SUCCESS 0
#define INVALID_EXPRESSION 1
#define DIVISION_BY_ZERO 2

// check the operation in expression
int checkOperator(char ch)
{
    if(ch=='+' || ch=='-' || ch=='*' || ch=='/')
    return 1;
    return 0;
}

// calculate the expression
int calculate(char expression[], long long* result)
{
    int i = 0;
    long long res = 0;
    long long curr = 0;
    long long num;
    char op = '+';
    int hasNumber = 0;
    while(expression[i]!='\0' && expression[i]!='\n')
    {
        if(expression[i]==' ' || expression[i]=='\t')
        {
            i++;
            continue;
        }
        if(expression[i]<'0' || expression[i]>'9')
        {
            return INVALID_EXPRESSION;
        }
        num = 0;
        while(expression[i]>='0' && expression[i]<='9')
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
                return DIVISION_BY_ZERO;
            }
            curr /= num;
        }
    
    while(expression[i]==' ' || expression[i]=='\t')
    {
        i++;
    }
    if(checkOperator(expression[i]))
    {
        op = expression[i];
        i++;
        hasNumber = 0;
    }
    else if(expression[i]!='\0' && expression[i]!='\n')
    {
        return INVALID_EXPRESSION;
    }
}
    if(!hasNumber)
    {
        return INVALID_EXPRESSION;
    }
    res += curr;
    *result = res;
    return SUCCESS;
}

int main() 
{
    char expression[1000];
    long long result;
    int status;
    printf("Enter the expression: ");
    fgets(expression, sizeof(expression), stdin);

    status = calculate(expression, &result);

    if (status==SUCCESS) 
    printf("Result: %lld\n", result);

    else if (status==INVALID_EXPRESSION) 
    printf("Error: Invalid Expression!\n");

    else if (status==DIVISION_BY_ZERO) 
    printf("Error: Division By Zero!\n");

    printf("status code: %d\n", status);
}