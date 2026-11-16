#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024
#define MAX_NAME_LENGTH 50
#define MAX_INPUT_LENGTH 4096
#define COMMAND_COUNT 11

enum InputStatus { INPUT_EOF=0, INPUT_OK=1, INPUT_TOO_LONG=2 };
enum NodeType { FILE_TYPE=1, DIRECTORY_TYPE=2 };
enum Command {
    COMMAND_EXIT=1, COMMAND_LS=2, COMMAND_PWD=3, COMMAND_DF=4,
    COMMAND_MKDIR=5, COMMAND_CREATE=6, COMMAND_READ=7,
    COMMAND_DELETE=8, COMMAND_RMDIR=9, COMMAND_CD=10,
    COMMAND_WRITE=11
};

typedef struct FreeBlock FreeBlock;
typedef struct FileNode FileNode;
typedef struct VirtualFileSystem VirtualFileSystem;

struct FreeBlock {
    int index;
    FreeBlock *next;
    FreeBlock *prev;
};

struct FileNode {
    char name[MAX_NAME_LENGTH+1];
    int type;
    int size;
    int blockCount;
    int *blockPointers;
    FileNode *parent;
    FileNode *next;
    FileNode *prev;
    FileNode *children;
};

struct VirtualFileSystem {
    unsigned char (*virtualDisk)[BLOCK_SIZE];
    FreeBlock *freeHead;
    FreeBlock *freeTail;
    FreeBlock *blockNodes;
    int freeCount;
    FileNode *root;
    FileNode *cwd;
};

/* Function declarations */
int stringLength(const char text[]);
int stringCompare(const char first[], const char second[]);
void stringCopy(char destination[], const char source[]);
int isSpaceCharacter(char ch);
int readLine(char line[]);

FileNode *createNode(char name[], int type, FileNode *parent);
void insertChild(FileNode *directory, FileNode *node);
void removeChild(FileNode *directory, FileNode *node);
FileNode *findChild(FileNode *directory, const char name[]);
void freeTree(FileNode *node);

void appendFreeBlock(VirtualFileSystem *vfs, int index);
int takeFreeBlock(VirtualFileSystem *vfs);
void releaseBlocks(VirtualFileSystem *vfs, FileNode *file);

int initializeVfs(VirtualFileSystem *vfs);
void destroyVfs(VirtualFileSystem *vfs);

void printPath(FileNode *node);
void addEntry(VirtualFileSystem *vfs, char name[], int type);
void writeFile(VirtualFileSystem *vfs, FileNode *file, const char content[], int length);
void readFile(VirtualFileSystem *vfs, FileNode *file);
void deleteFile(VirtualFileSystem *vfs, char name[]);
void removeDirectory(VirtualFileSystem *vfs, char name[]);
void changeDirectory(VirtualFileSystem *vfs, char name[]);
void listDirectory(VirtualFileSystem *vfs);
void showDiskUsage(VirtualFileSystem *vfs);
void runWriteCommand(VirtualFileSystem *vfs, char line[]);
int executeCommand(VirtualFileSystem *vfs, char line[]);

/* String helper functions */
int stringLength(const char text[]) {
    int length=0;
    while(text[length]!='\0') length++;
    return length;
}

int stringCompare(const char first[], const char second[]) {
    int i=0;
    while(first[i]!='\0' && second[i]!='\0') {
        if(first[i]!=second[i]) return first[i]-second[i];
        i++;
    }
    return first[i]-second[i];
}

void stringCopy(char destination[], const char source[]) {
    int i=0;
    while(source[i]!='\0') {
        destination[i]=source[i];
        i++;
    }
    destination[i]='\0';
}

int isSpaceCharacter(char ch) {
    return ch==' ' || ch=='\t' || ch=='\r' || ch=='\n';
}

/* Input handling */
int readLine(char line[]) {
    int ch, length=0, tooLong=0;

    while(1) {
        ch=getchar();
        if(ch==EOF || ch=='\n') break;

        if(length<MAX_INPUT_LENGTH-1) line[length++]=(char)ch;
        else tooLong=1;
    }

    if(ch==EOF && length==0 && !tooLong) return INPUT_EOF;

    line[length]='\0';

    if(length>0 && line[length-1]=='\r') line[length-1]='\0';

    if(tooLong) return INPUT_TOO_LONG;
    return INPUT_OK;
}

/* File and directory node operations */
FileNode *createNode(char name[], int type, FileNode *parent) {
    FileNode *node;
    int length=stringLength(name);

    if(length>MAX_NAME_LENGTH) return NULL;

    node=malloc(sizeof(FileNode));
    if(node==NULL) return NULL;

    stringCopy(node->name, name);
    node->type=type;
    node->size=0;
    node->blockCount=0;
    node->blockPointers=NULL;
    node->parent=parent;
    node->next=node;
    node->prev=node;
    node->children=NULL;

    return node;
}

void insertChild(FileNode *directory, FileNode *node) {
    if(directory->children==NULL) {
        directory->children=node;
        node->next=node;
        node->prev=node;
    } else {
        FileNode *last=directory->children->prev;
        last->next=node;
        node->prev=last;
        node->next=directory->children;
        directory->children->prev=node;
    }
    node->parent=directory;
}

void removeChild(FileNode *directory, FileNode *node) {
    if(directory==NULL || node==NULL || directory->children==NULL) 
    return;

    if(node->next==node) 
    {
        directory->children=NULL;
    }
    else 
    {
        node->prev->next=node->next;
        node->next->prev=node->prev;

        if(directory->children==node) directory->children=node->next;
    }

    node->next=node;
    node->prev=node;
    node->parent=NULL;
}

FileNode *findChild(FileNode *directory, const char name[]) {
    FileNode *current;

    if(directory==NULL || directory->children==NULL) 
    return NULL;

    current=directory->children;
    do {
        if(stringCompare(current->name, name)==0) return current;
        current=current->next;
    } while(current!=directory->children);

    return NULL;
}

void freeTree(FileNode *node) {
    FileNode *first, *current, *next;

    if(node==NULL) return;

    if(node->type==DIRECTORY_TYPE && node->children!=NULL) {
        first=node->children;
        current=first->next;

        while(current!=first) {
            next=current->next;
            freeTree(current);
            current=next;
        }

        freeTree(first);
    }

    free(node->blockPointers);
    free(node);
}

/* Free block doubly linked list */
void appendFreeBlock(VirtualFileSystem *vfs, int index) {
    FreeBlock *node=&vfs->blockNodes[index];

    node->index=index;
    node->next=NULL;
    node->prev=vfs->freeTail;

    if(vfs->freeTail==NULL) vfs->freeHead=node;
    else vfs->freeTail->next=node;

    vfs->freeTail=node;
    vfs->freeCount++;
}

int takeFreeBlock(VirtualFileSystem *vfs) {
    FreeBlock *node;
    int index;

    if(vfs->freeHead==NULL) return -1;

    node=vfs->freeHead;
    index=node->index;
    vfs->freeHead=node->next;

    if(vfs->freeHead==NULL) vfs->freeTail=NULL;
    else vfs->freeHead->prev=NULL;

    node->next=NULL;
    node->prev=NULL;
    vfs->freeCount--;

    return index;
}

void releaseBlocks(VirtualFileSystem *vfs, FileNode *file) {
    int i;

    for(i=0; i<file->blockCount; i++) {
        appendFreeBlock(vfs, file->blockPointers[i]);
    }

    free(file->blockPointers);
    file->blockPointers=NULL;
    file->blockCount=0;
    file->size=0;
}

/* Virtual file system initialization and cleanup */
int initializeVfs(VirtualFileSystem *vfs) {
    int i;
    char rootName[]="/";

    vfs->virtualDisk=NULL;
    vfs->freeHead=NULL;
    vfs->freeTail=NULL;
    vfs->blockNodes=NULL;
    vfs->freeCount=0;
    vfs->root=NULL;
    vfs->cwd=NULL;

    vfs->virtualDisk=malloc(NUM_BLOCKS*sizeof(*vfs->virtualDisk));
    if(vfs->virtualDisk==NULL) {
        printf("Error: Unable to allocate virtual disk.\n");
        return 0;
    }

    vfs->blockNodes=malloc(NUM_BLOCKS*sizeof(FreeBlock));
    if(vfs->blockNodes==NULL) {
        printf("Error: Unable to allocate free block nodes.\n");
        destroyVfs(vfs);
        return 0;
    }

    for(i=0; i<NUM_BLOCKS; i++) appendFreeBlock(vfs, i);

    vfs->root=createNode(rootName, DIRECTORY_TYPE, NULL);
    if(vfs->root==NULL) {
        printf("Error: Unable to create root directory.\n");
        destroyVfs(vfs);
        return 0;
    }

    vfs->cwd=vfs->root;
    return 1;
}

void destroyVfs(VirtualFileSystem *vfs) {
    freeTree(vfs->root);
    free(vfs->blockNodes);
    free(vfs->virtualDisk);

    vfs->root=NULL;
    vfs->cwd=NULL;
    vfs->blockNodes=NULL;
    vfs->virtualDisk=NULL;
    vfs->freeHead=NULL;
    vfs->freeTail=NULL;
    vfs->freeCount=0;
}

/* Path handling */
void printPath(FileNode *node) {
    if(node==NULL) return;

    if(node->parent==NULL) {
        printf("/");
        return;
    }

    printPath(node->parent);

    if(node->parent->parent!=NULL) printf("/");
    printf("%s", node->name);
}

/* Create file or directory */
void addEntry(VirtualFileSystem *vfs, char name[], int type) {
    FileNode *node;
    int length=stringLength(name), i;

    if(length==0 || length>MAX_NAME_LENGTH) {
        printf("Error: Invalid name length.\n");
        return;
    }

    if(stringCompare(name, ".")==0 || stringCompare(name, "..")==0) {
        printf("Error: Reserved name cannot be used.\n");
        return;
    }

    for(i=0; i<length; i++) {
        if(name[i]=='/') {
            printf("Error: Name cannot contain '/'.\n");
            return;
        }
    }

    if(findChild(vfs->cwd, name)!=NULL) {
        printf("Error: An entry with this name already exists.\n");
        return;
    }

    node=createNode(name, type, vfs->cwd);
    if(node==NULL) {
        printf("Error: Memory allocation failed.\n");
        return;
    }

    insertChild(vfs->cwd, node);

    if(type==DIRECTORY_TYPE) printf("Directory '%s' created successfully.\n", name);
    else printf("File '%s' created successfully.\n", name);
}

/* Write file */
void writeFile(VirtualFileSystem *vfs, FileNode *file, const char content[], int length) {
    int requiredBlocks, *newPointers=NULL;
    int i, copied=0;

    if(file==NULL || file->type!=FILE_TYPE) {
        printf("Error: Invalid file.\n");
        return;
    }

    if(length<0) {
        printf("Error: Invalid content length.\n");
        return;
    }

    requiredBlocks=(length+BLOCK_SIZE-1)/BLOCK_SIZE;

    if(requiredBlocks>vfs->freeCount+file->blockCount) {
        printf("Error: Not enough disk space.\n");
        return;
    }

    if(requiredBlocks>0) {
        newPointers=malloc(requiredBlocks*sizeof(int));
        if(newPointers==NULL) {
            printf("Error: Memory allocation failed.\n");
            return;
        }
    }

    releaseBlocks(vfs, file);

    for(i=0; i<requiredBlocks; i++) {
        newPointers[i]=takeFreeBlock(vfs);

        if(newPointers[i]==-1) {
            int j;
            for(j=0; j<i; j++) appendFreeBlock(vfs, newPointers[j]);
            free(newPointers);
            printf("Error: Disk allocation failed.\n");
            return;
        }
    }

    for(i=0; i<requiredBlocks; i++) {
        int bytesToCopy=length-copied, j;
        if(bytesToCopy>BLOCK_SIZE) bytesToCopy=BLOCK_SIZE;

        for(j=0; j<bytesToCopy; j++) {
            vfs->virtualDisk[newPointers[i]][j]=(unsigned char)content[copied];
            copied++;
        }
    }

    file->blockPointers=newPointers;
    file->blockCount=requiredBlocks;
    file->size=length;

    printf("File '%s' written successfully. (%d bytes)\n", file->name, length);
}

/* Read file */
void readFile(VirtualFileSystem *vfs, FileNode *file) {
    int remaining, i;

    if(file==NULL || file->type!=FILE_TYPE) {
        printf("Error: Invalid file.\n");
        return;
    }

    if(file->size==0) {
        printf("File is empty.\n");
        return;
    }

    remaining=file->size;
    printf("Contents of '%s':\n", file->name);

    for(i=0; i<file->blockCount && remaining>0; i++) {
        int bytesToRead=remaining, j;
        if(bytesToRead>BLOCK_SIZE) bytesToRead=BLOCK_SIZE;

        for(j=0; j<bytesToRead; j++) {
            putchar(vfs->virtualDisk[file->blockPointers[i]][j]);
        }

        remaining-=bytesToRead;
    }

    printf("\n");
}

/* Delete file */
void deleteFile(VirtualFileSystem *vfs, char name[]) {
    FileNode *file=findChild(vfs->cwd, name);

    if(file==NULL) {
        printf("Error: File not found.\n");
        return;
    }

    if(file->type!=FILE_TYPE) {
        printf("Error: '%s' is not a file.\n", name);
        return;
    }

    releaseBlocks(vfs, file);
    removeChild(vfs->cwd, file);
    free(file);

    printf("File '%s' deleted successfully.\n", name);
}

/* Remove directory */
void removeDirectory(VirtualFileSystem *vfs, char name[]) {
    FileNode *directory=findChild(vfs->cwd, name);

    if(directory==NULL) {
        printf("Error: Directory not found.\n");
        return;
    }

    if(directory->type!=DIRECTORY_TYPE) {
        printf("Error: '%s' is not a directory.\n", name);
        return;
    }

    if(directory->children!=NULL) {
        printf("Error: Directory is not empty.\n");
        return;
    }

    removeChild(vfs->cwd, directory);
    free(directory);

    printf("Directory '%s' removed successfully.\n", name);
}

/* Change current directory */
void changeDirectory(VirtualFileSystem *vfs, char name[]) {
    FileNode *directory;

    if(stringCompare(name, "/")==0) {
        vfs->cwd=vfs->root;
        return;
    }

    if(stringCompare(name, "..")==0) {
        if(vfs->cwd->parent!=NULL) vfs->cwd=vfs->cwd->parent;
        return;
    }

    if(stringCompare(name, ".")==0) return;

    directory=findChild(vfs->cwd, name);

    if(directory==NULL) {
        printf("Error: Directory not found.\n");
        return;
    }

    if(directory->type!=DIRECTORY_TYPE) {
        printf("Error: '%s' is not a directory.\n", name);
        return;
    }

    vfs->cwd=directory;
}

/* List directory contents */
void listDirectory(VirtualFileSystem *vfs) {
    FileNode *current;

    if(vfs->cwd->children==NULL) {
        printf("Directory is empty.\n");
        return;
    }

    printf("%-25s %-15s %-10s\n", "Name", "Type", "Size");
    printf("--------------------------------------------------\n");

    current=vfs->cwd->children;

    do {
        if(current->type==DIRECTORY_TYPE) {
            printf("%-25s %-15s %-10s\n", current->name, "Directory", "-");
        } else {
            printf("%-25s %-15s %-10d\n", current->name, "File", current->size);
        }

        current=current->next;
    } while(current!=vfs->cwd->children);
}

/* Show disk usage */
void showDiskUsage(VirtualFileSystem *vfs) {
    int usedBlocks=NUM_BLOCKS-vfs->freeCount;

    printf("Total disk blocks : %d\n", NUM_BLOCKS);
    printf("Free disk blocks  : %d\n", vfs->freeCount);
    printf("Used disk blocks  : %d\n", usedBlocks);
    printf("Block size        : %d bytes\n", BLOCK_SIZE);
    printf("Total disk space  : %d bytes\n", NUM_BLOCKS*BLOCK_SIZE);
    printf("Free disk space   : %d bytes\n", vfs->freeCount*BLOCK_SIZE);
    printf("Used disk space   : %d bytes\n", usedBlocks*BLOCK_SIZE);
}

/* Write command parser */
void runWriteCommand(VirtualFileSystem *vfs, char line[]) {
    int position=5, nameStart, nameLength;
    int contentStart, contentEnd, contentLength;
    char name[MAX_NAME_LENGTH+1], quote;
    FileNode *file;

    while(isSpaceCharacter(line[position])) position++;

    nameStart=position;

    while(line[position]!='\0' && !isSpaceCharacter(line[position])) position++;

    nameLength=position-nameStart;

    if(nameLength==0 || nameLength>MAX_NAME_LENGTH) {
        printf("Error: Invalid file name.\n");
        return;
    }

    {
        int i;
        for(i=0; i<nameLength; i++) name[i]=line[nameStart+i];
        name[nameLength]='\0';
    }

    while(line[position]==' ' || line[position]=='\t') position++;

    if(line[position]!='"' && line[position]!='\'') {
        printf("Error: Content must be enclosed in quotes.\n");
        return;
    }

    quote=line[position];
    contentStart=position+1;
    contentEnd=stringLength(line)-1;

    while(contentEnd>=contentStart &&
          (line[contentEnd]==' ' || line[contentEnd]=='\t' || line[contentEnd]=='\r')) {
        contentEnd--;
    }

    if(contentEnd<contentStart || line[contentEnd]!=quote) {
        printf("Error: Missing closing quote.\n");
        return;
    }

    contentLength=contentEnd-contentStart;
    file=findChild(vfs->cwd, name);

    if(file==NULL) {
        printf("Error: File not found.\n");
        return;
    }

    if(file->type!=FILE_TYPE) {
        printf("Error: '%s' is not a file.\n", name);
        return;
    }

    {
        char content[MAX_INPUT_LENGTH];
        int i;

        for(i=0; i<contentLength; i++) content[i]=line[contentStart+i];
        content[contentLength]='\0';

        writeFile(vfs, file, content, contentLength);
    }
}

/* Command execution */
int executeCommand(VirtualFileSystem *vfs, char line[]) {
    char command[MAX_INPUT_LENGTH], argument[MAX_INPUT_LENGTH];
    int position=0, commandLength=0, argumentLength=0;
    int commandNumber=0, i;

    char commandNames[COMMAND_COUNT][10]={
        "exit", "ls", "pwd", "df", "mkdir", "create",
        "read", "delete", "rmdir", "cd", "write"
    };

    int commandValues[COMMAND_COUNT]={
        COMMAND_EXIT, COMMAND_LS, COMMAND_PWD, COMMAND_DF,
        COMMAND_MKDIR, COMMAND_CREATE, COMMAND_READ,
        COMMAND_DELETE, COMMAND_RMDIR, COMMAND_CD, COMMAND_WRITE
    };

    while(isSpaceCharacter(line[position])) position++;
    if(line[position]=='\0') return 1;

    while(line[position]!='\0' && !isSpaceCharacter(line[position])) {
        command[commandLength++]=line[position++];
    }
    command[commandLength]='\0';

    for(i=0; i<COMMAND_COUNT; i++) {
        if(stringCompare(command, commandNames[i])==0) {
            commandNumber=commandValues[i];
            break;
        }
    }

    if(commandNumber==0) {
        printf("Error: Unknown command '%s'.\n", command);
        return 1;
    }

    while(isSpaceCharacter(line[position])) position++;

    if(commandNumber==COMMAND_WRITE) {
        runWriteCommand(vfs, line);
        return 1;
    }

    while(line[position]!='\0') {
        if(argumentLength>=MAX_INPUT_LENGTH-1) {
            printf("Error: Argument too long.\n");
            return 1;
        }
        argument[argumentLength++]=line[position++];
    }
    argument[argumentLength]='\0';

    while(argumentLength>0 && isSpaceCharacter(argument[argumentLength-1])) {
        argument[--argumentLength]='\0';
    }

    if(commandNumber==COMMAND_EXIT) {
        if(argumentLength!=0) {
            printf("Error: exit takes no arguments.\n");
            return 1;
        }
        return 0;
    }

    if(commandNumber==COMMAND_LS) {
        if(argumentLength!=0) {
            printf("Error: ls takes no arguments.\n");
            return 1;
        }
        listDirectory(vfs);
        return 1;
    }

    if(commandNumber==COMMAND_PWD) {
        if(argumentLength!=0) {
            printf("Error: pwd takes no arguments.\n");
            return 1;
        }
        printPath(vfs->cwd);
        printf("\n");
        return 1;
    }

    if(commandNumber==COMMAND_DF) {
        if(argumentLength!=0) {
            printf("Error: df takes no arguments.\n");
            return 1;
        }
        showDiskUsage(vfs);
        return 1;
    }

    if(argumentLength==0) {
        printf("Error: This command requires an argument.\n");
        return 1;
    }

    for(i=0; i<argumentLength; i++) {
        if(isSpaceCharacter(argument[i])) {
            printf("Error: Invalid argument.\n");
            return 1;
        }
    }

    switch(commandNumber) {
        case COMMAND_MKDIR:
            addEntry(vfs, argument, DIRECTORY_TYPE);
            break;

        case COMMAND_CREATE:
            addEntry(vfs, argument, FILE_TYPE);
            break;

        case COMMAND_READ: {
            FileNode *file=findChild(vfs->cwd, argument);

            if(file==NULL) printf("Error: File not found.\n");
            else if(file->type!=FILE_TYPE) printf("Error: '%s' is not a file.\n", argument);
            else readFile(vfs, file);
            break;
        }

        case COMMAND_DELETE:
            deleteFile(vfs, argument);
            break;

        case COMMAND_RMDIR:
            removeDirectory(vfs, argument);
            break;

        case COMMAND_CD:
            changeDirectory(vfs, argument);
            break;

        default:
            printf("Error: Invalid command.\n");
            break;
    }

    return 1;
}

/* Main function */
int main() {
    VirtualFileSystem vfs;
    char line[MAX_INPUT_LENGTH];

    if(!initializeVfs(&vfs)) return 1;

    printf("========================================\n");
    printf("       VIRTUAL FILE SYSTEM (VFS)\n");
    printf("========================================\n");
    printf("Type 'help' to see available commands.\n");
    printf("Type 'exit' to close the file system.\n\n");

    while(1) {
        int inputStatus, continueRunning;

        printf("VFS> ");
        fflush(stdout);

        inputStatus=readLine(line);

        if(inputStatus==INPUT_EOF) {
            printf("\n");
            break;
        }

        if(inputStatus==INPUT_TOO_LONG) {
            printf("Error: Input line is too long.\n");
            continue;
        }

        if(stringCompare(line, "help")==0) {
            printf("\nAvailable commands:\n");
            printf("mkdir <name>        - Create a directory\n");
            printf("create <name>       - Create a file\n");
            printf("write <name> \"text\" - Write to a file\n");
            printf("read <name>         - Read a file\n");
            printf("delete <name>       - Delete a file\n");
            printf("rmdir <name>        - Remove an empty directory\n");
            printf("ls                  - List directory contents\n");
            printf("cd <name>           - Change directory\n");
            printf("cd ..               - Move to parent directory\n");
            printf("cd /                - Move to root directory\n");
            printf("pwd                 - Print current directory\n");
            printf("df                  - Show disk usage\n");
            printf("exit                - Exit the file system\n\n");
            continue;
        }

        continueRunning=executeCommand(&vfs, line);
        if(!continueRunning) break;
    }

    destroyVfs(&vfs);
    printf("Virtual file system closed successfully.\n");

    return 0;
}
