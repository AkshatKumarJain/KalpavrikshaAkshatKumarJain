#include <stdio.h>

struct User
{
    int id;
    char name[50];
    int age;
};

void createUser()
{
    FILE *fp;
    struct User user;
    int id;

    printf("Enter ID: ");
    scanf("%d", &id);

    fp = fopen("users.txt", "r");
    if (fp!=NULL)
    {
        while (fscanf(fp, "%d %s %d", &user.id, user.name, &user.age) != EOF)
        {
            if (user.id == id)
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
    scanf("%s", user.name);

    printf("Enter Age: ");
    scanf("%d", &user.age);

    fprintf(fp, "%d %s %d\n", user.id, user.name, user.age);
    fclose(fp);
    printf("User added successfully.\n");
}

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
    while (fscanf(fp, "%d %s %d", &user.id, user.name, &user.age)!=EOF)
    {
        printf("%d\t%s\t%d\n", user.id, user.name, user.age);
    }
    fclose(fp);
}

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

    while (fscanf(fp, "%d %s %d", &user.id, user.name, &user.age)!=EOF)
    {
        if (user.id==id)
        {
            printf("Enter new name: ");
            scanf("%s", user.name);
            printf("Enter new age: ");
            scanf("%d", &user.age);
            flag = 1;
        }

        fprintf(temp, "%d %s %d\n", user.id, user.name, user.age);
    }

    fclose(fp);
    fclose(temp);
    remove("users.txt");
    rename("temp.txt", "users.txt");
    if (flag == 1)
    printf("User updated successfully.\n");
    else
    printf("User not found.\n");
}

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

    while (fscanf(fp, "%d %s %d", &user.id, user.name, &user.age) != EOF)
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

int main()
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
        printf("1. Create User\n");
        printf("2. Read Users\n");
        printf("3. Update User\n");
        printf("4. Delete User\n");
        printf("5. Exit\n");

        printf("Enter your choice: ");
        scanf("%c", &x);

        if(x<'1' || x>'5')
        {
        printf("\nPlease enter correct choice\n");
        continue;
        }

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
        else
        printf("Invalid choice.\n");
    }
}

