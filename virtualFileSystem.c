#include <stdio.h>
#include <stdlib.h>

#define BLOCK_SIZE 512
#define NUM_BLOCKS 1024
#define MAX_NAME_LENGTH 50
#define MAX_INPUT_LENGTH 4096
#define COMMAND_COUNT 12

enum InputStatus { INPUT_EOF=0, INPUT_OK=1, INPUT_TOO_LONG=2 };
enum NodeType { NODE_FILE=1, NODE_DIRECTORY=2 };
enum Command {
    COMMAND_EXIT=1, COMMAND_LS=2, COMMAND_PWD=3, COMMAND_DF=4,
    COMMAND_MKDIR=5, COMMAND_CREATE=6, COMMAND_READ=7,
    COMMAND_DELETE=8, COMMAND_RMDIR=9, COMMAND_CD=10,
    COMMAND_WRITE=11, COMMAND_HELP=12
};

typedef struct FreeBlockEntry FreeBlockEntry;
typedef struct FsEntry FsEntry;
typedef struct VirtualFileSystemState VirtualFileSystemState;

struct FreeBlockEntry {
    int index;
    FreeBlockEntry *next;
    FreeBlockEntry *prev;
};

struct FsEntry {
    char name[MAX_NAME_LENGTH+1];
    int type;
    int byte_size;
    int block_count;
    int *block_ids;
    FsEntry *parent;
    // Children are kept in a circular doubly linked sibling list. 
    FsEntry *next;
    FsEntry *prev;
    FsEntry *children;
};

struct VirtualFileSystemState {
    unsigned char (*disk_blocks)[BLOCK_SIZE];
    FreeBlockEntry *available_head;
    FreeBlockEntry *available_tail;
    FreeBlockEntry *free_block_entries;
    int available_count;
    FsEntry *root;
    FsEntry *working_directory;
};

// Function declarations
int find_command(const char command[]);
void display_help(void);
int validate_no_argument_command(int command_number, const char argument[]);
void execute_command(VirtualFileSystemState *fs, int command_number, char argument[]);
int text_length(const char text[]);
int text_compare(const char first[], const char second[]);
void text_copy(char destination[], const char source[]);
int is_space(char ch);
int read_input_line(char line[]);

FsEntry *create_entry(char name[], int type, FsEntry *parent);
void attach_child(FsEntry *folder, FsEntry *entry);
void detach_child(FsEntry *folder, FsEntry *entry);
FsEntry *find_child(FsEntry *folder, const char name[]);
void free_entry_tree(FsEntry *entry);

void append_free_block(VirtualFileSystemState *fs, int index);
int take_free_block(VirtualFileSystemState *fs);
void release_file_blocks(VirtualFileSystemState *fs, FsEntry *file_entry);

int initialize_filesystem(VirtualFileSystemState *fs);
void destroy_filesystem(VirtualFileSystemState *fs);

void print_working_path(FsEntry *entry);
void create_named_entry(VirtualFileSystemState *fs, char name[], int type);
void write_file_contents(VirtualFileSystemState *fs, FsEntry *file_entry, const char content[], int length);
void display_file_contents(VirtualFileSystemState *fs, FsEntry *file_entry);
void delete_named_file(VirtualFileSystemState *fs, char name[]);
void remove_empty_directory(VirtualFileSystemState *fs, char name[]);
void change_working_directory(VirtualFileSystemState *fs, char name[]);
void list_working_directory(VirtualFileSystemState *fs);
void display_disk_usage(VirtualFileSystemState *fs);
void parse_write_command(VirtualFileSystemState *fs, char line[], int argument_start);
int dispatch_command(VirtualFileSystemState *fs, char line[]);

// String helper functions 
int text_length(const char text[]) {
    int length=0;
    while(text[length]!='\0') length++;
    return length;
}

int text_compare(const char first[], const char second[]) {
    int i=0;
    while(first[i]!='\0' && second[i]!='\0') {
        if(first[i]!=second[i]) return first[i]-second[i];
        i++;
    }
    return first[i]-second[i];
}

void text_copy(char destination[], const char source[]) {
    int i=0;
    while(source[i]!='\0') {
        destination[i]=source[i];
        i++;
    }
    destination[i]='\0';
}

int is_space(char ch) {
    return ch==' ' || ch=='\t' || ch=='\r' || ch=='\n';
}

// Input handling
int read_input_line(char line[]) {
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

int find_command(const char command[]) {
    char command_names[COMMAND_COUNT][10]={
        "exit", "ls", "pwd", "df", "mkdir", "create",
        "read", "delete", "rmdir", "cd", "write", "help"
    };

    int command_values[COMMAND_COUNT]={
        COMMAND_EXIT, COMMAND_LS, COMMAND_PWD, COMMAND_DF,
        COMMAND_MKDIR, COMMAND_CREATE, COMMAND_READ,
        COMMAND_DELETE, COMMAND_RMDIR, COMMAND_CD,
        COMMAND_WRITE, COMMAND_HELP
    };

    int i;

    for(i=0; i<COMMAND_COUNT; i++) {
        if(text_compare(command, command_names[i])==0) {
            return command_values[i];
        }
    }

    return 0;
}

void display_help(void) {
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
    printf("help                - Show available commands\n");
    printf("exit                - Exit the file system\n\n");
}

int validate_no_argument_command(int command_number, const char argument[]) {
    if(command_number==COMMAND_EXIT ||
       command_number==COMMAND_HELP ||
       command_number==COMMAND_LS ||
       command_number==COMMAND_PWD ||
       command_number==COMMAND_DF) {

        if(argument[0]!='\0') {
            if(command_number==COMMAND_EXIT)
                printf("Error: exit takes no arguments.\n");
            else if(command_number==COMMAND_HELP)
                printf("Error: help takes no arguments.\n");
            else if(command_number==COMMAND_LS)
                printf("Error: ls takes no arguments.\n");
            else if(command_number==COMMAND_PWD)
                printf("Error: pwd takes no arguments.\n");
            else
                printf("Error: df takes no arguments.\n");

            return 0;
        }
    }

    return 1;
}

void execute_command(VirtualFileSystemState *fs, int command_number, char argument[]) {
    int i;

    if(argument[0]=='\0') {
        printf("Error: This command requires an argument.\n");
        return;
    }

    for(i=0; argument[i]!='\0'; i++) {
        if(is_space(argument[i])) {
            printf("Error: Invalid argument.\n");
            return;
        }
    }

    switch(command_number) {
        case COMMAND_MKDIR:
            create_named_entry(fs, argument, NODE_DIRECTORY);
            break;

        case COMMAND_CREATE:
            create_named_entry(fs, argument, NODE_FILE);
            break;

        case COMMAND_READ: {
            FsEntry *file_entry=find_child(fs->working_directory, argument);

            if(file_entry==NULL)
                printf("Error: File not found.\n");
            else if(file_entry->type!=NODE_FILE)
                printf("Error: '%s' is not a file.\n", argument);
            else
                display_file_contents(fs, file_entry);

            break;
        }

        case COMMAND_DELETE:
            delete_named_file(fs, argument);
            break;

        case COMMAND_RMDIR:
            remove_empty_directory(fs, argument);
            break;

        case COMMAND_CD:
            change_working_directory(fs, argument);
            break;

        default:
            printf("Error: Invalid command.\n");
            break;
    }
}

/* File and directory entry operations */
FsEntry *create_entry(char name[], int type, FsEntry *parent) {
    FsEntry *entry;
    int length=text_length(name);

    if(length>MAX_NAME_LENGTH) return NULL;

    entry=malloc(sizeof(FsEntry));
    if(entry==NULL) return NULL;

    text_copy(entry->name, name);
    entry->type=type;
    entry->byte_size=0;
    entry->block_count=0;
    entry->block_ids=NULL;
    entry->parent=parent;
    entry->next=entry;
    entry->prev=entry;
    entry->children=NULL;

    return entry;
}

void attach_child(FsEntry *folder, FsEntry *entry) {
    if(folder->children==NULL) {
        folder->children=entry;
        entry->next=entry;
        entry->prev=entry;
    } else {
        FsEntry *last=folder->children->prev;
        last->next=entry;
        entry->prev=last;
        entry->next=folder->children;
        folder->children->prev=entry;
    }
    entry->parent=folder;
}

void detach_child(FsEntry *folder, FsEntry *entry) {
    if(folder==NULL || entry==NULL || folder->children==NULL)
    return;

    if(entry->next==entry)
    {
        folder->children=NULL;
    }
    else 
    {
        entry->prev->next=entry->next;
        entry->next->prev=entry->prev;

        if(folder->children==entry) folder->children=entry->next;
    }

    entry->next=entry;
    entry->prev=entry;
    entry->parent=NULL;
}

FsEntry *find_child(FsEntry *folder, const char name[]) {
    FsEntry *cursor;

    if(folder==NULL || folder->children==NULL)
    return NULL;

    cursor=folder->children;
    do {
        if(text_compare(cursor->name, name)==0) return cursor;
        cursor=cursor->next;
    } while(cursor!=folder->children);

    return NULL;
}

void free_entry_tree(FsEntry *entry) {
    FsEntry *first, *cursor, *next;

    if(entry==NULL) return;

    if(entry->type==NODE_DIRECTORY && entry->children!=NULL) {
        first=entry->children;
        cursor=first->next;

        while(cursor!=first) {
            next=cursor->next;
            free_entry_tree(cursor);
            cursor=next;
        }

        free_entry_tree(first);
    }

    free(entry->block_ids);
    free(entry);
}

/* Free block doubly linked list */
void append_free_block(VirtualFileSystemState *fs, int index) {
    FreeBlockEntry *entry=&fs->free_block_entries[index];

    entry->index=index;
    entry->next=NULL;
    entry->prev=fs->available_tail;

    if(fs->available_tail==NULL) fs->available_head=entry;
    else fs->available_tail->next=entry;

    fs->available_tail=entry;
    fs->available_count++;
}

int take_free_block(VirtualFileSystemState *fs) {
    FreeBlockEntry *entry;
    int index;

    if(fs->available_head==NULL) return -1;

    entry=fs->available_head;
    index=entry->index;
    fs->available_head=entry->next;

    if(fs->available_head==NULL) fs->available_tail=NULL;
    else fs->available_head->prev=NULL;

    entry->next=NULL;
    entry->prev=NULL;
    fs->available_count--;

    return index;
}

void release_file_blocks(VirtualFileSystemState *fs, FsEntry *file_entry) {
    int i;

    for(i=0; i<file_entry->block_count; i++) {
        append_free_block(fs, file_entry->block_ids[i]);
    }

    free(file_entry->block_ids);
    file_entry->block_ids=NULL;
    file_entry->block_count=0;
    file_entry->byte_size=0;
}

/* Virtual file system initialization and cleanup */
int initialize_filesystem(VirtualFileSystemState *fs) {
    int i;
    char root_name[]="/";

    fs->disk_blocks=NULL;
    fs->available_head=NULL;
    fs->available_tail=NULL;
    fs->free_block_entries=NULL;
    fs->available_count=0;
    fs->root=NULL;
    fs->working_directory=NULL;

    fs->disk_blocks=malloc(NUM_BLOCKS*sizeof(*fs->disk_blocks));
    if(fs->disk_blocks==NULL) {
        printf("Error: Unable to allocate virtual disk.\n");
        return 0;
    }

    fs->free_block_entries=malloc(NUM_BLOCKS*sizeof(FreeBlockEntry));
    if(fs->free_block_entries==NULL) {
        printf("Error: Unable to allocate free block nodes.\n");
        destroy_filesystem(fs);
        return 0;
    }

    for(i=0; i<NUM_BLOCKS; i++) append_free_block(fs, i);

    fs->root=create_entry(root_name, NODE_DIRECTORY, NULL);
    if(fs->root==NULL) {
        printf("Error: Unable to create root directory.\n");
        destroy_filesystem(fs);
        return 0;
    }

    fs->working_directory=fs->root;
    return 1;
}

void destroy_filesystem(VirtualFileSystemState *fs) {
    free_entry_tree(fs->root);
    free(fs->free_block_entries);
    free(fs->disk_blocks);

    fs->root=NULL;
    fs->working_directory=NULL;
    fs->free_block_entries=NULL;
    fs->disk_blocks=NULL;
    fs->available_head=NULL;
    fs->available_tail=NULL;
    fs->available_count=0;
}

/* Path handling */
void print_working_path(FsEntry *entry) {
    if(entry==NULL) return;

    if(entry->parent==NULL) {
        printf("/");
        return;
    }

    print_working_path(entry->parent);

    if(entry->parent->parent!=NULL) printf("/");
    printf("%s", entry->name);
}

/* Create file or directory */
void create_named_entry(VirtualFileSystemState *fs, char name[], int type) {
    FsEntry *entry;
    int length=text_length(name), i;

    if(length==0 || length>MAX_NAME_LENGTH) {
        printf("Error: Invalid name length.\n");
        return;
    }

    if(text_compare(name, ".")==0 || text_compare(name, "..")==0) {
        printf("Error: Reserved name cannot be used.\n");
        return;
    }

    for(i=0; i<length; i++) {
        if(name[i]=='/') {
            printf("Error: Name cannot contain '/'.\n");
            return;
        }
    }

    if(find_child(fs->working_directory, name)!=NULL) {
        printf("Error: An entry with this name already exists.\n");
        return;
    }

    entry=create_entry(name, type, fs->working_directory);
    if(entry==NULL) {
        printf("Error: Memory allocation failed.\n");
        return;
    }

    attach_child(fs->working_directory, entry);

    if(type==NODE_DIRECTORY) printf("Directory '%s' created successfully.\n", name);
    else printf("File '%s' created successfully.\n", name);
}

// Write file 
void write_file_contents(VirtualFileSystemState *fs, FsEntry *file_entry, const char content[], int length) {
    int required_blocks, *new_block_ids=NULL;
    int i, copied=0;
    int old_block_count, reused_blocks, allocated_count=0;

    if(file_entry==NULL || file_entry->type!=NODE_FILE) {
        printf("Error: Invalid file.\n");
        return;
    }

    if(length<0) {
        printf("Error: Invalid content length.\n");
        return;
    }

    required_blocks = (length+BLOCK_SIZE-1)/BLOCK_SIZE;

    old_block_count = file_entry->block_count;

    if(required_blocks > fs->available_count+old_block_count) {
        printf("Error: Not enough disk space.\n");
        return;
    }

    if(required_blocks>0) {
        new_block_ids = malloc(required_blocks*sizeof(int));

        if(new_block_ids == NULL) {
            printf("Error: Memory allocation failed.\n");
            return;
        }
    }

    reused_blocks = required_blocks<old_block_count ? required_blocks : old_block_count;

    for(i=0; i<reused_blocks; i++) {
        new_block_ids[i]=file_entry->block_ids[i];
    }

    for(i=reused_blocks; i<required_blocks; i++) {
        new_block_ids[i]=take_free_block(fs);

        if(new_block_ids[i]==-1) {
            int j;

            for(j=reused_blocks; j<allocated_count+reused_blocks; j++) {
                append_free_block(fs, new_block_ids[j]);
            }

            free(new_block_ids);
            printf("Error: Disk allocation failed.\n");
            return;
        }

        allocated_count++;
    }

    for(i=0; i<required_blocks; i++) {
        int bytes_to_copy=length-copied, j;

        if(bytes_to_copy>BLOCK_SIZE) bytes_to_copy=BLOCK_SIZE;

        for(j=0; j<bytes_to_copy; j++) {
            fs->disk_blocks[new_block_ids[i]][j] = (unsigned char)content[copied];
            copied++;
        }
    }

    for(i=required_blocks; i<old_block_count; i++) {
        append_free_block(fs, file_entry->block_ids[i]);
    }

    free(file_entry->block_ids);

    file_entry->block_ids=new_block_ids;
    file_entry->block_count=required_blocks;
    file_entry->byte_size=length;

    printf("File '%s' written successfully. (%d bytes)\n", file_entry->name, length);
}


/* Read file */
void display_file_contents(VirtualFileSystemState *fs, FsEntry *file_entry) {
    int remaining, i;

    if(file_entry==NULL || file_entry->type!=NODE_FILE) {
        printf("Error: Invalid file.\n");
        return;
    }

    if(file_entry->byte_size==0) {
        printf("File is empty.\n");
        return;
    }

    remaining=file_entry->byte_size;
    printf("Contents of '%s':\n", file_entry->name);

    for(i=0; i<file_entry->block_count && remaining>0; i++) {
        int bytesToRead=remaining, j;
        if(bytesToRead>BLOCK_SIZE) bytesToRead=BLOCK_SIZE;

        for(j=0; j<bytesToRead; j++) {
            putchar(fs->disk_blocks[file_entry->block_ids[i]][j]);
        }

        remaining-=bytesToRead;
    }

    printf("\n");
}

/* Delete file */
void delete_named_file(VirtualFileSystemState *fs, char name[]) {
    FsEntry *file_entry=find_child(fs->working_directory, name);

    if(file_entry==NULL) {
        printf("Error: File not found.\n");
        return;
    }

    if(file_entry->type!=NODE_FILE) {
        printf("Error: '%s' is not a file.\n", name);
        return;
    }

    release_file_blocks(fs, file_entry);
    detach_child(fs->working_directory, file_entry);
    free(file_entry);

    printf("File '%s' deleted successfully.\n", name);
}

/* Remove directory */
void remove_empty_directory(VirtualFileSystemState *fs, char name[]) {
    FsEntry *folder=find_child(fs->working_directory, name);

    if(folder==NULL) {
        printf("Error: Directory not found.\n");
        return;
    }

    if(folder->type!=NODE_DIRECTORY) {
        printf("Error: '%s' is not a directory.\n", name);
        return;
    }

    if(folder->children!=NULL) {
        printf("Error: Directory is not empty.\n");
        return;
    }

    detach_child(fs->working_directory, folder);
    free(folder);

    printf("Directory '%s' removed successfully.\n", name);
}

/* Change current directory */
void change_working_directory(VirtualFileSystemState *fs, char name[]) {
    FsEntry *folder;

    if(text_compare(name, "/")==0) {
        fs->working_directory=fs->root;
        return;
    }

    if(text_compare(name, "..")==0) {
        if(fs->working_directory->parent!=NULL) fs->working_directory=fs->working_directory->parent;
        return;
    }

    if(text_compare(name, ".")==0) return;

    folder=find_child(fs->working_directory, name);

    if(folder==NULL) {
        printf("Error: Directory not found.\n");
        return;
    }

    if(folder->type!=NODE_DIRECTORY) {
        printf("Error: '%s' is not a directory.\n", name);
        return;
    }

    fs->working_directory=folder;
}

/* List directory contents */
void list_working_directory(VirtualFileSystemState *fs) {
    FsEntry *cursor;

    if(fs->working_directory->children==NULL) {
        printf("Directory is empty.\n");
        return;
    }

    printf("%-25s %-15s %-10s\n", "Name", "Type", "Size");
    printf("--------------------------------------------------\n");

    cursor=fs->working_directory->children;

    do {
        if(cursor->type==NODE_DIRECTORY) {
            printf("%-25s %-15s %-10s\n", cursor->name, "Directory", "-");
        } else {
            printf("%-25s %-15s %-10d\n", cursor->name, "File", cursor->byte_size);
        }

        cursor=cursor->next;
    } while(cursor!=fs->working_directory->children);
}

/* Show disk usage */
void display_disk_usage(VirtualFileSystemState *fs) {
    int used_blocks=NUM_BLOCKS-fs->available_count;

    printf("Total disk blocks : %d\n", NUM_BLOCKS);
    printf("Free disk blocks  : %d\n", fs->available_count);
    printf("Used disk blocks  : %d\n", used_blocks);
    printf("Block size        : %d bytes\n", BLOCK_SIZE);
    printf("Total disk space  : %d bytes\n", NUM_BLOCKS*BLOCK_SIZE);
    printf("Free disk space   : %d bytes\n", fs->available_count*BLOCK_SIZE);
    printf("Used disk space   : %d bytes\n", used_blocks*BLOCK_SIZE);
}

// Write command parser

void parse_write_command(VirtualFileSystemState *fs, char line[], int argument_start) {
    int position=argument_start, name_start, name_length;
    int content_start, content_end, content_length;
    char name[MAX_NAME_LENGTH+1], quote;
    FsEntry *file_entry;

    while(is_space(line[position])) position++;

    name_start=position;

    while(line[position]!='\0' && !is_space(line[position])) position++;

    name_length=position-name_start;

    if(name_length==0 || name_length>MAX_NAME_LENGTH) {
        printf("Error: Invalid file name.\n");
        return;
    }

    {
        int i;
        for(i=0; i<name_length; i++) name[i]=line[name_start+i];
        name[name_length]='\0';
    }

    while(line[position]==' ' || line[position]=='\t') position++;

    if(line[position]!='"' && line[position]!='\'') {
        printf("Error: Content must be enclosed in quotes.\n");
        return;
    }

    quote=line[position];
    content_start=position+1;
    content_end=content_start;

    while(line[content_end]!='\0' && line[content_end]!=quote) {
        content_end++;
    }

    if(line[content_end]!=quote) {
        printf("Error: Missing closing quote.\n");
        return;
    }

    position=content_end+1;

    while(is_space(line[position])) position++;

    if(line[position]!='\0') {
        printf("Error: Unexpected characters after quoted content.\n");
        return;
    }

    content_length=content_end-content_start;

    file_entry=find_child(fs->working_directory, name);

    if(file_entry==NULL) {
        printf("Error: File not found.\n");
        return;
    }

    if(file_entry->type!=NODE_FILE) {
        printf("Error: '%s' is not a file.\n", name);
        return;
    }

    {
        char content[MAX_INPUT_LENGTH];
        int i;

        for(i=0; i<content_length; i++) {
            content[i]=line[content_start+i];
        }

        content[content_length]='\0';

        write_file_contents(fs, file_entry, content, content_length);
    }
}

// Command execution
int dispatch_command(VirtualFileSystemState *fs, char line[]) {
    char command[MAX_INPUT_LENGTH], argument[MAX_INPUT_LENGTH];
    int position=0, command_length=0, argument_length=0;
    int command_number;

    while(is_space(line[position])) position++;

    if(line[position]=='\0') return 1;

    while(line[position]!='\0' && !is_space(line[position])) {
        command[command_length++]=line[position++];
    }

    command[command_length]='\0';

    command_number=find_command(command);

    if(command_number==0) {
        printf("Error: Unknown command '%s'.\n", command);
        return 1;
    }

    while(is_space(line[position])) position++;

    if(command_number==COMMAND_WRITE) {
        parse_write_command(fs, line, position);
        return 1;
    }

    while(line[position]!='\0') {
        if(argument_length>=MAX_INPUT_LENGTH-1) {
            printf("Error: Argument too long.\n");
            return 1;
        }

        argument[argument_length++]=line[position++];
    }

    argument[argument_length]='\0';

    while(argument_length>0 && is_space(argument[argument_length-1])) {
        argument[--argument_length]='\0';
    }

    if(!validate_no_argument_command(command_number, argument)) {
        return 1;
    }

    if(command_number==COMMAND_EXIT) return 0;

    if(command_number==COMMAND_HELP) {
        display_help();
        return 1;
    }

    if(command_number==COMMAND_LS) {
        list_working_directory(fs);
        return 1;
    }

    if(command_number==COMMAND_PWD) {
        print_working_path(fs->working_directory);
        printf("\n");
        return 1;
    }

    if(command_number==COMMAND_DF) {
        display_disk_usage(fs);
        return 1;
    }

    execute_command(fs, command_number, argument);

    return 1;
}

/* Main function */
int main() {
    VirtualFileSystemState fs;
    char line[MAX_INPUT_LENGTH];

    if(!initialize_filesystem(&fs)) return 1;

    printf("========================================\n");
    printf("       VIRTUAL FILE SYSTEM (VFS)\n");
    printf("========================================\n");
    printf("Type 'help' to see available commands.\n");
    printf("Type 'exit' to close the file system.\n\n");

    while(1) {
        int input_status, continue_running;

        printf("VFS> ");
        fflush(stdout);

        input_status=read_input_line(line);

        if(input_status==INPUT_EOF) {
            printf("\n");
            break;
        }

        if(input_status==INPUT_TOO_LONG) 
        {
            printf("Error: Input line is too long.\n");
            continue;
        }
        continue_running=dispatch_command(&fs, line);
        if(!continue_running) break;
    }

    destroy_filesystem(&fs);
    printf("Virtual file system closed successfully.\n");

    return 0;
}
