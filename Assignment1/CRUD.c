#include <stdio.h>

// define a User structure to store the properties of user as object
struct User
{
    int id;
    char name[50];
    int age;
};

// check if the name entered is valid or not
int isNameValid(char name[])
{
    if(name[0]=='\0')
    return 0;
    for(int i=0;name[i]!='\0';i++)
    {
        if(!((name[i]>='A'&& name[i]<='Z') || (name[i]>='a'&&name[i]<='z')))
        return 0;
    }
    return 1;
}

// create new user with unique id
void createUser()
{
    FILE *fp;
    struct User user;
    int id;

    printf("Enter ID: ");
    scanf("%d", &id);

    fp = fopen("users.txt", "r");
    if(fp!=NULL)
    {
        while (fscanf(fp, "%d %49s %d", &user.id, user.name, &user.age) == 3 )
        {
            if(user.id==id)
            {
                fclose(fp);
                printf("ID must be unique!\n");
                return;
            }
        }
        fclose(fp);
    }

    fp = fopen("users.txt", "a");
    if(fp==NULL)
    {
        printf("File could not be opened.\n");
        return;
    }
    user.id = id;
    printf("Enter Name: ");
    scanf("%49s", user.name);

    if(!isNameValid(user.name))
    {
        printf("Invalid name!\n");
        fclose(fp);
        return;
    }

    printf("Enter Age: ");
    scanf("%d", &user.age);
    if(user.age<1 || user.age>110)
    {
        printf("Invalid age!\n");
        fclose(fp);
        return;
    }

    fprintf(fp, "%d %s %d\n", user.id, user.name, user.age);
    fclose(fp);
    printf("User added successfully.\n");
}

// read the user on the basis of unique id
void readUsers()
{
    FILE *fp;
    struct User user;
    fp = fopen("users.txt", "r");
    if(fp==NULL)
    {
        printf("File could not be opened.\n");
        return;
    }
    printf("\nID\tName\tAge\n");
    printf("-------------------------\n");
    while (fscanf(fp, "%d %49s %d", &user.id, user.name, &user.age)==3)
    {
        printf("%d\t%s\t%d\n", user.id, user.name, user.age);
    }
    fclose(fp);
}

// update the properties(name, age) of user
void updateUser()
{
    FILE *fp;
    FILE *temp;
    struct User user;
    int id;
    int flag = 0;
    fp = fopen("users.txt", "r");
    temp = fopen("temp.txt", "w");
    if(fp==NULL || temp==NULL)
    {
        printf("File could not be opened.\n");
        return;
    }
    printf("Enter ID to update: ");
    scanf("%d", &id);

    while(fscanf(fp, "%d %49s %d", &user.id, user.name, &user.age)==3)
    {
        if(user.id==id)
        {
            printf("Enter new name: ");
            scanf("%49s", user.name);

            if(!isNameValid(user.name))
            {
                printf("Invalid name!\n");
                fclose(fp);
                fclose(temp);
                remove("temp.txt");
                return;
            }
            printf("Enter new age: ");
            scanf("%d", &user.age);
            if(user.age<1 || user.age>110)
            {
                printf("Invalid age!\n");
                fclose(fp);
                fclose(temp);
                remove("temp.txt");
                return;
            }
            flag = 1;
        }

        fprintf(temp, "%d %s %d\n", user.id, user.name, user.age);
    }

    fclose(fp);
    fclose(temp);
    remove("users.txt");
    rename("temp.txt", "users.txt");
    if (flag==1)
    printf("User updated successfully.\n");
    else
    printf("User not found.\n");
}

// delete a user
void deleteUser()
{
    FILE *fp;
    FILE *temp;
    struct User user;
    int id;
    int flag = 0;

    fp = fopen("users.txt", "r");
    temp = fopen("temp.txt", "w");

    if (fp==NULL || temp == NULL)
    {
        printf("File could not be opened.\n");
        return;
    }

    printf("Enter ID to delete: ");
    scanf("%d", &id);

    while (fscanf(fp, "%d %49s %d", &user.id, user.name, &user.age) == 3)
    {
        if (user.id==id)
        {
            flag = 1;
            continue;
        }

        fprintf(temp, "%d %s %d\n", user.id, user.name, user.age);
    }

    fclose(fp);
    fclose(temp);

    remove("users.txt");
    rename("temp.txt", "users.txt");

    if (flag == 1)
    printf("User deleted successfully.\n");
    else
    printf("User not found.\n");
}

// show the menu to user
void showMenu()
{
    printf("1. Create User\n");
    printf("2. Read Users\n");
    printf("3. Update User\n");
    printf("4. Delete User\n");
    printf("5. Exit\n");
}

// function to open or create file and start operations
int start()
{
    FILE *fp;
    fp = fopen("users.txt", "a");
    if(fp==NULL)
    {
        printf("File could not be created.");
        return 0;
    }
    fclose(fp);
    while(1)
    {
        char x;

        printf("Enter 6 to show menu.\n");

        printf("Enter your choice: ");
        scanf(" %c", &x);

        int c;
        while ((c = getchar()) != '\n' && c != EOF);

        if (x=='1')
        createUser();
        else if (x=='2')
        readUsers();
        else if (x=='3')
        updateUser();
        else if (x == '4')
        deleteUser();
        else if (x=='5')
        break;
        else if(x=='6')
        showMenu();
        else
        printf("Invalid choice.\n");
    }
}

int main()
{
    start();
}

