#include <stdio.h>
#include <stdlib.h>

#define NAME_LEN 50
#define MAX_REPORTS 100
#define MAX_TOP_LEVEL 100

struct Employee {
    int id;
    char name[NAME_LEN];
    char designation[NAME_LEN];
    struct Employee *manager;
    struct Employee *reports[MAX_REPORTS];
    int reportCount;
};

struct Employee *topLevel[MAX_TOP_LEVEL];
int topLevelCount = 0;
int employeeCount = 0;

// Function declarations 
int  validateName(char name[]);
struct Employee *findEmployee(struct Employee *employee, int id);
struct Employee *findEmployeeInOrganization(int id);
int countUnder(struct Employee *employee);
void displayHierarchy(struct Employee *employee, int level);
void addEmployee(void);
void removeEmployee(void);
void displayEmployeeHierarchy(void);
void displayCompleteHierarchy(void);
void freeOrganization(struct Employee *employee);
int takeInput(void);
void showMenu();


// Validate name
int validateName(char name[])
{
    int i = 0;
    if (name[0] == '\0')
    return 0;
    while (name[i] != '\0') {
        if (!((name[i] >= 'A' && name[i] <= 'Z') || (name[i] >= 'a' && name[i] <= 'z'))) {
            return 0;
        }
        i++;
    }

    return 1;
}

// Find employee recursively
struct Employee *findEmployee(struct Employee *employee, int id)
{
    if (employee == NULL)
        return NULL;

    if (employee->id == id)
        return employee;

    for (int i = 0; i < employee->reportCount; i++) {
        struct Employee *found =
            findEmployee(employee->reports[i], id);

        if (found != NULL)
            return found;
    }

    return NULL;
}

// Find employee in the complete organization
struct Employee *findEmployeeInOrganization(int id)
{
    for (int i = 0; i < topLevelCount; i++) {
        struct Employee *found = findEmployee(topLevel[i], id);

        if (found != NULL)
            return found;
    }

    return NULL;
}

// Count all employees under an employee recursively
int countUnder(struct Employee *employee)
{
    if (employee == NULL)
    return 0;

    int total = 0;

    for (int i=0; i<employee->reportCount; i++) {
        total += 1 + countUnder(employee->reports[i]);
    }

    return total;
}

// Display hierarchy recursively
void displayHierarchy(struct Employee *employee, int level)
{
    if(employee == NULL)
    return;
    for(int i = 0; i < level * 4; i++)
    {
        printf(" ");
    }
    printf("%d - %s - %s\n",employee->id, employee->name, employee->designation);

    for (int i=0;i<employee->reportCount;i++) {
        displayHierarchy(employee->reports[i], level + 1);
    }
}

// Add an employee 
void addEmployee()
{
    int id, managerId = 0;
    char name[NAME_LEN], designation[NAME_LEN];

    printf("Enter Employee ID: ");

    if (scanf("%d", &id) != 1 || id <= 0) {
        printf("Invalid employee ID.\n");
        return;
    }

    if (findEmployeeInOrganization(id) != NULL) {
        printf("Employee ID already exists.\n");
        return;
    }

    struct Employee *manager = NULL;

    printf("Enter Reporting Manager ID (0 for no manager): ");

    if (scanf("%d", &managerId) != 1 || managerId < 0) {
        printf("Invalid manager ID.\n");
        return;
    }

    if (managerId == 0) {
        if (topLevelCount >= MAX_TOP_LEVEL) {
            printf("Maximum top-level employees reached.\n");
            return;
        }

        printf("This employee will be a top-level employee.\n");
    } else {
        manager = findEmployeeInOrganization(managerId);

        if (manager == NULL) {
            printf("Reporting manager not found.\n");
            return;
        }

        if (manager->reportCount >= MAX_REPORTS) {
            printf("This manager cannot have more direct reports.\n");
            return;
        }
    }

    printf("Enter Name: ");

    if (scanf("%49s", name) != 1)
        return;

    if (!validateName(name)) {
        printf("Invalid name.\n");
        return;
    }

    printf("Enter Designation: ");

    if (scanf("%49s", designation) != 1)
        return;

    if (!validateName(designation)) {
        printf("Invalid designation.\n");
        return;
    }

    struct Employee *employee = malloc(sizeof(struct Employee));

    if (employee == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }

    employee->id = id;
   
    int i = 0;

    while(name[i]!='\0') 
    {
        employee->name[i] = name[i];
        i++;
    }
    employee->name[i]='\0';

    i = 0;

    while(designation[i]!='\0') 
    {
        employee->designation[i] = designation[i];
        i++;
    }
    employee->designation[i] = '\0';

    employee->manager = manager;
    employee->reportCount = 0;

    if(manager==NULL)
    {
        topLevel[topLevelCount] = employee;
        topLevelCount++;
    } 
    else
    {
        manager->reports[manager->reportCount] = employee;
        manager->reportCount++;
    }

    employeeCount++;

    printf("Employee added successfully.\n");
}

// Choose sibling with the fewest direct reports
struct Employee *chooseTarget(struct Employee *manager, struct Employee *removed)
{
    struct Employee *target = NULL;

    for (int i=0;i<manager->reportCount;i++) {
        struct Employee *sibling = manager->reports[i];

        if(sibling==removed)
        continue;

        if(target==NULL || sibling->reportCount<target->reportCount) {
            target = sibling;
        }
    }

    if(target==NULL)
    target = manager;

    return target;
}

// Remove employee from manager's reports
void detachFromManager(struct Employee *employee)
{
    struct Employee *manager = employee->manager;
    int position = 0;

    while(position<manager->reportCount && manager->reports[position]!=employee) {
        position++;
    }

    if (position == manager->reportCount)
    return;

    for (int i=position;i<manager->reportCount-1;i++) {
        manager->reports[i] = manager->reports[i + 1];
    }

    manager->reportCount--;
}

// Remove employee and transfer direct reports 
void removeEmployee()
{
    int id;

    printf("Enter Employee ID to remove: ");

    if (scanf("%d", &id) != 1 || id <= 0) {
        printf("Invalid employee ID.\n");
        return;
    }

    struct Employee *employee = findEmployeeInOrganization(id);

    if (employee==NULL) {
        printf("Employee not found.\n");
        return;
    }

    if (employee->manager == NULL) {
        printf("A top-level employee cannot be removed.\n");
        return;
    }

    struct Employee *manager = employee->manager;
    struct Employee *target = NULL;

    if (employee->reportCount > 0) {
        target = chooseTarget(manager, employee);

        int availableReports = MAX_REPORTS-target->reportCount;

        if(target == manager)
        availableReports++;

        if(employee->reportCount > availableReports) 
        {
            printf("Cannot remove employee: target has insufficient capacity.\n");
            return;
        }
    }

    detachFromManager(employee);

    if (target != NULL) {
        while (employee->reportCount > 0) {
            struct Employee *report = employee->reports[employee->reportCount - 1];

            employee->reportCount--;

            report->manager = target;
            target->reports[target->reportCount] = report;
            target->reportCount++;
        }
    }

    free(employee);
    employeeCount--;

    printf("Employee removed successfully.\n");

    if (target != NULL) {
        printf("Direct reports transferred to %d - %s.\n", target->id, target->name);
    }
}

// Display selected employee's hierarchy
void displayEmployeeHierarchy(void)
{
    int id;
    printf("Enter Employee ID: ");
    if (scanf("%d", &id) != 1 || id <= 0) {
        printf("Invalid employee ID.\n");
        return;
    }
    struct Employee *employee = findEmployeeInOrganization(id);
    if (employee == NULL) {
        printf("Employee not found.\n");
        return;
    }

    printf("\n===== EMPLOYEE HIERARCHY =====\n");

    displayHierarchy(employee, 0);

    printf("Total Employees Under %d: %d\n", id, countUnder(employee));
}

// Display complete organization 
void displayCompleteHierarchy()
{
    if(topLevelCount == 0) 
    {
        printf("Organization is empty.\n");
        return;
    }
    printf("\n===== ORGANIZATION HIERARCHY =====\n");

    for(int i=0;i<topLevelCount;i++) 
    {
        displayHierarchy(topLevel[i], 0);
    }

    printf("Total Employees: %d\n", employeeCount);
}

// Free memory recursively
void freeOrganization(struct Employee *employee)
{
    if(employee==NULL)
    return;
    for(int i=0;i<employee->reportCount;i++) {
        freeOrganization(employee->reports[i]);
    }
    free(employee);
}

void showMenu(){
    printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
    printf("1. Add Employee\n");
    printf("2. Remove Employee\n");
    printf("3. Display Employee Hierarchy\n");
    printf("4. Display Complete Hierarchy\n");
    printf("5. Exit\n");
}

int takeInput()
{
    int choice;
    printf("Enter your choice: ");

    if (scanf("%d", &choice) != 1) {
        printf("Invalid choice. Please restart the menu selection.\n");
        return 1;
    }

    switch (choice) {
        case 1:
            addEmployee();
            break;

        case 2:
            removeEmployee();
            break;

        case 3:
            displayEmployeeHierarchy();
            break;

        case 4:
            displayCompleteHierarchy();
            break;

        case 5:
            printf("Exiting program.\n");
            break;

        default:
            printf("Invalid choice. Enter 1 to 5.\n");
        }
        return choice;
}

// Main function
int main()
{
    int choice = 0;
    while(choice!=5)
    {
        showMenu();
        choice = takeInput();
    }

    for (int i = 0; i < topLevelCount; i++) {
        freeOrganization(topLevel[i]);
    }

    return 0;
}