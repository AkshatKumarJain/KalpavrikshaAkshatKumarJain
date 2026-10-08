#include <stdio.h>
#define MAX_STUDENTS 100

// Structure to store student details
struct Student
{
    int rollNumber;
    char name[50];
    int marks1;
    int marks2;
    int marks3;
};

// validate the name
int validateName(char name[])
{
    int i = 0;
    if(name[0]=='\0')
    return 0;
    while(name[i]!='\0')
    {
        if(!((name[i]>='A' && name[i] <= 'Z') || (name[i]>='a' && name[i]<='z')))
        return 0;
        i++;
    }
    return 1;
}

// check if the rool number is unique
int doesRollNoExists(struct Student students[], int currentIndex, int rollNumber)
{
    for(int i=0;i<currentIndex;i++)
    {
        if(students[i].rollNumber==rollNumber)
        return 1;
    }

    return 0;
}

// calculate total marks
int calculateTotalMarks(struct Student student)
{
    return student.marks1+student.marks2+student.marks3;
}

// calculate average marks
float calculateAverageMarks(int total)
{
    return total/3.0;
}

// assign grade to students
char calculateGrade(float average)
{
    if(average>=85)
    return 'A';
    else if(average>=70)
    return 'B';
    else if(average>=50)
    return 'C';
    else if(average>=35)
    return 'D';
    else
    return 'F';
}

// print performance pattern of students on the basis of stars
void printPerformance(char grade)
{
    int stars;
    if(grade=='A')
    stars = 5;
    else if(grade=='B')
    stars = 4;
    else if(grade=='C')
    stars = 3;
    else if(grade=='D')
    stars = 2;
    else
    stars = 0;

    for(int i=0;i<stars;i++)
    {
        printf("*");
    }
    printf("\n");
}

// Recursion function to print roll numbers
void printRollNumbers(struct Student students[], int i, int n)
{
    if(i==n)
    return;
    printf("%d", students[i].rollNumber);
    if(i<n-1)
    printf(" ");
    printRollNumbers(students, i+1, n);
}

// take inputs for all students
void takeInputsForStudents(struct Student students[], int n)
{
    for(int i=0;i<n;i++)
    {
        printf("\nEnter details for student %d:\n", i+1);
        while(1)
        {
            printf("Enter roll number: ");
            scanf("%d", &students[i].rollNumber);
            if (students[i].rollNumber<=0)
            printf("Invalid roll number!\n");
            else if(doesRollNoExists(students, i, students[i].rollNumber))
            printf("Roll number already exists. Enter a unique roll number.\n");
            else
            break;
        }

        while(1)
        {
            printf("Enter name: ");
            scanf("%49s", students[i].name);
            if(validateName(students[i].name))
            break;
            printf("Invalid name!\n");
        }

        while(1)
        {
            printf("Enter marks for 1st subjects: ");
            scanf("%d", &students[i].marks1);

            printf("Enter marks for 2nd subjects: ");
            scanf("%d", &students[i].marks2);

            printf("Enter marks for 3rd subjects: ");
            scanf("%d", &students[i].marks3);

            if (students[i].marks1>=0 && students[i].marks1<=100 && students[i].marks2>=0 && students[i].marks2 <= 100 && students[i].marks3 >= 0 && students[i].marks3 <= 100)
            break;
            printf("Invalid marks. Each mark must be between 0 and 100.\n");
        }
    }
}

void processStudentDetails(struct Student students[], int n)
{
    for (int i=0;i<n;i++)
    {
        int total = calculateTotalMarks(students[i]);
        float average = calculateAverageMarks(total);
        char grade = calculateGrade(average);

        printf("Roll No: %d\n", students[i].rollNumber);
        printf("Name: %s\n", students[i].name);
        printf("Total Marks: %d\n", total);
        printf("Average Marks: %.2f\n", average);
        printf("Grade: %c\n", grade);

        // Skip performance pattern for grade F
        if (average < 35)
        {
            printf("*****************");
            printf("\n");
            continue;
        }

        printf("Performance: ");
        printPerformance(grade);

        printf("*****************");
        printf("\n");
    }
    printf("List of Roll Numbers (via recursion): ");
    printRollNumbers(students, 0, n);
    printf("\n");
}

int main()
{
    struct Student students[MAX_STUDENTS];
    int n;

    // demonstration of variable scope
    printf("Enter number of students: ");
    scanf("%d", &n);

    if (n<1 || n>MAX_STUDENTS)
    {
        printf("Invalid number of students.\n");
        return 1;
    }

    takeInputsForStudents(students, n);
    printf("\n");

    processStudentDetails(students, n);
}