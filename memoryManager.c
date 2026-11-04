#include <stdio.h>
#include <stdlib.h>

#define MIN_POOL 1
#define MAX_POOL 1000000
#define INITIAL_CAPACITY 4
#define MAX_ALLOCATION_ID 2147483647

enum Status{
    FREE=0,ALLOCATED=1
};
enum Strategy{
    FIRST_FIT=1,BEST_FIT=2,WORST_FIT=3
};
enum Menu{
    ALLOCATE=1,RELEASE=2,RESIZE=3,MAP=4,STATS=5,CHANGE=6,EXIT=7
};

struct Segment{
    int begin;
    int length;
    int allocationId;
    int state;
};

struct MemorySystem{
    unsigned char *pool;
    struct Segment *segments;
    int segmentCount;
    int segmentCapacity;
    int totalSize;
    int nextId;
    int allocationMethod;
    int fragmentationFlag;
    int failedSize;
};

int getNumber(int minValue,int maxValue,const char *message);
void zeroMemory(struct MemorySystem *system,int begin,int length);
int createManager(struct MemorySystem *system,int totalSize);
void destroyManager(struct MemorySystem *system);
void printMenu();
const char *getStrategyName(int allocationMethod);
void setStrategy(struct MemorySystem *system);
int addSegment(struct MemorySystem *system,int pos,struct Segment block);
void deleteSegment(struct MemorySystem *system,int pos);
int selectFirstFit(struct MemorySystem *system,int length);
int selectBestFit(struct MemorySystem *system,int length);
int selectWorstFit(struct MemorySystem *system,int length);
int selectFreeSegment(struct MemorySystem *system,int length);
int searchAllocation(struct MemorySystem *system,int allocationId);
int searchSegmentStart(struct MemorySystem *system,int begin);
int occupySegment(struct MemorySystem *system,int pos,int length,int allocationId);
void combineFreeSegments(struct MemorySystem *system);
int reduceAllocation(struct MemorySystem *system,int pos,int updatedSize);
int expandAllocation(struct MemorySystem *system,int pos,int updatedSize);
int calculateFreeSpace(struct MemorySystem *system);
void checkFragmentation(struct MemorySystem *system,int length);
void showInvalidId(struct MemorySystem *system,int allocationId);
void requestMemory(struct MemorySystem *system);
void freeMemory(struct MemorySystem *system);
void changeMemorySize(struct MemorySystem *system);
void printMemoryLayout(struct MemorySystem *system);
void printMemoryStats(struct MemorySystem *system);

int getNumber(int minValue,int maxValue,const char *message)
{
    int number, digits, valid, ch, started, ended, digit;
    while(1)
    {
        number = 0;
        digits = 0; 
        valid = 1;
        started = 0;
        ended = 0;
        printf("%s", message);
        while((ch=getchar())!='\n'&&ch!=EOF){
            if(ch==' '||ch=='\t'||ch=='\r'){
                if(started){
                    ended=1;
                }
            }
            else if(ch>='0'&&ch<='9')
            {
                if(ended)
                {
                    valid=0;
                }
                started=1;
                digits++;
                digit=ch-'0';
                if(number>(maxValue-digit)/10)
                {
                    valid=0;
                }
                else if(valid)
                {
                    number=number*10+digit;
                }
            }
            else
            {
                valid=0;
            }
        }
        if(ch==EOF&&digits==0){
            exit(0);
        }
        if(valid&&digits>0&&number>=minValue&&number<=maxValue){
            return number;
        }
        printf("Invalid input. Enter a number between %d and %d.\n",minValue,maxValue);
    }
}

int createManager(struct MemorySystem *system, int totalSize)
{
    system->pool = NULL;
    system->segments = NULL;
    system->pool = malloc((size_t)totalSize*sizeof(unsigned char));
    system->segments = malloc(INITIAL_CAPACITY*sizeof(struct Segment));
    if(system->pool == NULL || system->segments==NULL)
    {
        free(system->pool);
        free(system->segments);
        system->pool = NULL;
        system->segments = NULL;
        return 0;
    }
    system->segments[0].begin = 0;
    system->segments[0].length = totalSize;
    system->segments[0].allocationId = 0;
    system->segments[0].state = FREE;
    system->segmentCount = 1;
    system->segmentCapacity = INITIAL_CAPACITY;
    system->totalSize = totalSize;
    system->nextId = 1;
    system->allocationMethod = FIRST_FIT;
    system->fragmentationFlag = 0;
    system->failedSize = 0;
    return 1;
}

void destroyManager(struct MemorySystem *system)
{
    free(system->pool);
    free(system->segments);
}

void zeroMemory(struct MemorySystem *system,int begin,int length)
{
    int i;
    for(i=0;i<length;i++)
    {
        system->pool[begin+i] = 0;
    }
}

void printMenu()
{
    printf("Menu:\n1. Allocate system\n2. Release system\n3. Resize allocation\n");
    printf("4. Display system map\n5. Display system statistics\n6. Change allocation allocationMethod\n7. Exit\n");
}

const char *getStrategyName(int allocationMethod)
{
    switch(allocationMethod){
        case FIRST_FIT:
            return "First Fit";
        case BEST_FIT:
            return "Best Fit";
        default:
            return "Worst Fit";
    }
}

void setStrategy(struct MemorySystem *system)
{
    system->allocationMethod = getNumber(FIRST_FIT,WORST_FIT,"Select allocation allocationMethod (1. First Fit, 2. Best Fit, 3. Worst Fit): ");
    printf("Strategy set to %s.\n",getStrategyName(system->allocationMethod));
}

int addSegment(struct MemorySystem *system,int pos,struct Segment block)
{
    int i;
    struct Segment *temp;
    if(system->segmentCount==system->segmentCapacity)
    {
        temp = realloc(system->segments,(size_t)2*system->segmentCapacity*sizeof(struct Segment));
        if(temp==NULL){
            return 0;
        }
        system->segments=temp;
        system->segmentCapacity *= 2;
    }
    for(i=system->segmentCount;i>pos;i--)
    {
        system->segments[i]=system->segments[i-1];
    }
    system->segments[pos] = block;
    system->segmentCount++;
    return 1;
}

void deleteSegment(struct MemorySystem *system, int pos)
{
    int i;

    for(i=pos; i<system->segmentCount-1; i++)
    {
        system->segments[i] = system->segments[i+1];
    }
    system->segmentCount--;
}

int selectFirstFit(struct MemorySystem *system, int length)
{
    int i;
    for(i=0; i<system->segmentCount; i++)
    {
        if(system->segments[i].state == FREE && system->segments[i].length>=length)
        {
            return i;
        }
    }
    return -1;
}

int selectBestFit(struct MemorySystem *system,int length)
{
    int i, bestPos = -1;

    for(i=0;i<system->segmentCount;i++)
    {
        if(system->segments[i].state == FREE && system->segments[i].length >= length)
        {
            if(bestPos==-1 || system->segments[i].length < system->segments[bestPos].length)
            {
                bestPos=i;
            }
        }
    }
    return bestPos;
}

int selectWorstFit(struct MemorySystem *system,int length)
{
    int i,bestPos = -1;
    for(i=0;i<system->segmentCount;i++)
    {
        if(system->segments[i].state == FREE&&system->segments[i].length>=length)
        {
            if(bestPos == -1 || system->segments[i].length > system->segments[bestPos].length)
            {
                bestPos = i;
            }
        }
    }
    return bestPos;
}

int selectFreeSegment(struct MemorySystem *system,int length){
    switch(system->allocationMethod){
        case FIRST_FIT:return selectFirstFit(system,length);
        case BEST_FIT:return selectBestFit(system,length);
        default:return selectWorstFit(system,length);
    }
}

int searchAllocation(struct MemorySystem *system,int allocationId)
{
    int i;
    for(i=0;i<system->segmentCount;i++)
    {
        if(system->segments[i].state == ALLOCATED&&system->segments[i].allocationId==allocationId){
            return i;
        }
    }
    return -1;
}

int searchSegmentStart(struct MemorySystem *system,int begin)
{
    int i;
    for(i=0;i<system->segmentCount;i++)
    {
        if(system->segments[i].begin == begin)
        {
            return i;
        }
    }
    return -1;
}

int occupySegment(struct MemorySystem *system,int pos,int length,int allocationId){
    int previousSize = system->segments[pos].length;
    struct Segment rest;
    if(previousSize > length)
    {
        rest.begin = system->segments[pos].begin + length;
        rest.length=previousSize-length;
        rest.allocationId=0;
        rest.state=FREE;
        if(!addSegment(system,pos+1,rest))
        {
            return 0;
        }
    }
    system->segments[pos].length = length;
    system->segments[pos].allocationId = allocationId;
    system->segments[pos].state = ALLOCATED;
    return 1;
}

void combineFreeSegments(struct MemorySystem *system)
{

    int i=0;
    while(i<system->segmentCount-1)
    {
        if(system->segments[i].state==FREE&&system->segments[i+1].state == FREE)
        {
            system->segments[i].length+=system->segments[i+1].length;
            deleteSegment(system,i+1);
        }
        else
        {
            i++;
        }
    }
}

int reduceAllocation(struct MemorySystem *system,int pos,int updatedSize)
{
    int changeAmount=system->segments[pos].length-updatedSize;
    int nextPos=pos+1;
    struct Segment freed;

    if(nextPos<system->segmentCount&&system->segments[nextPos].state == FREE)
    {
        system->segments[nextPos].begin-=changeAmount;
        system->segments[nextPos].length+=changeAmount;
    }
    else{
        freed.begin=system->segments[pos].begin+updatedSize;
        freed.length=changeAmount;
        freed.allocationId=0;
        freed.state=FREE;
        if(!addSegment(system,nextPos,freed)){
            return -1;
        }
    }

    zeroMemory(system,system->segments[pos].begin+updatedSize,changeAmount);
    system->segments[pos].length=updatedSize;
    return 1;
}

int expandAllocation(struct MemorySystem *system,int pos,int updatedSize){

    int i,j,allocationId,previousStart,previousSize,targetStart,oldIndex;
    int changeAmount=updatedSize-system->segments[pos].length;
    int nextPos=pos+1;

    if(nextPos<system->segmentCount&&system->segments[nextPos].state==FREE&&system->segments[nextPos].length>=changeAmount)
    {
        zeroMemory(system,system->segments[pos].begin+system->segments[pos].length,changeAmount);
        system->segments[pos].length=updatedSize;
        system->segments[nextPos].begin+=changeAmount;
        system->segments[nextPos].length-=changeAmount;
        if(system->segments[nextPos].length==0){
            deleteSegment(system,nextPos);
        }
        return 1;
    }

    j=selectFreeSegment(system,updatedSize);

    if(j==-1)
    {
        return 0;
    }

    allocationId=system->segments[pos].allocationId;
    previousStart=system->segments[pos].begin;
    previousSize=system->segments[pos].length;
    targetStart=system->segments[j].begin;

    for(i=0;i<previousSize;i++)
    {
        system->pool[targetStart+i]=system->pool[previousStart+i];
    }
    if(!occupySegment(system,j,updatedSize,allocationId))
    {
        return -1;
    }
    for(i=previousSize;i<updatedSize;i++)
    {
        system->pool[targetStart+i] = 0;
    }
    oldIndex=searchSegmentStart(system,previousStart);
    zeroMemory(system,previousStart,previousSize);
    system->segments[oldIndex].state = FREE;
    system->segments[oldIndex].allocationId = 0;
    combineFreeSegments(system);
    return 1;
}

int calculateFreeSpace(struct MemorySystem *system)
{
    int i,total=0;
    for(i=0;i<system->segmentCount;i++){
        if(system->segments[i].state==FREE)
        {
            total+=system->segments[i].length;
        }
    }
    return total;
}

void checkFragmentation(struct MemorySystem *system, int length)
{
    if(calculateFreeSpace(system) >= length)
    {
        system->fragmentationFlag=1;
        system->failedSize=length;
    }
}

void showInvalidId(struct MemorySystem *system,int allocationId)
{
    if(allocationId<system->nextId)
    {
        printf("Invalid request: allocation %d is already free.\n",allocationId);
    }
    else{
        printf("Invalid request: allocation ID %d does not exist.\n",allocationId);
    }
}

void requestMemory(struct MemorySystem *system)
{
    int i,pos,length,allocationId=system->nextId;
    length=getNumber(1,system->totalSize,"Requested length: ");
    if(system->nextId>=MAX_ALLOCATION_ID)
    {
        printf("Allocation failed: allocation ID limit reached.\n");
        return;
    }

    pos=selectFreeSegment(system,length);

    if(pos==-1)
    {
        printf("Allocation failed: no contiguous free block is large enough.\n");
        checkFragmentation(system,length);
        return;
    }
    if(!occupySegment(system,pos,length,allocationId))
    {
        printf("Allocation failed: could not grow the metadata array.\n");
        return;
    }
    for(i=0;i<length;i++)
    {
        system->pool[system->segments[pos].begin+i]=0;
    }
    system->nextId++;
    printf("Allocation successful. ID: %d, Start: %d, Size: %d\n",allocationId,system->segments[pos].begin,length);
}

void freeMemory(struct MemorySystem *system)
{
    int pos,allocationId;
    if(system->nextId==1)
    {
        printf("There are no allocations to release.\n");
        return;
    }
    allocationId=getNumber(1,system->nextId-1,"Allocation ID: ");
    pos=searchAllocation(system,allocationId);
    if(pos==-1)
    {
        showInvalidId(system,allocationId);
        return;
    }
    zeroMemory(system,system->segments[pos].begin,system->segments[pos].length);
    system->segments[pos].state=FREE;
    system->segments[pos].allocationId=0;
    combineFreeSegments(system);
    printf("Allocation released.\n");
}

void changeMemorySize(struct MemorySystem *system)
{
    int pos,allocationId,updatedSize,done;
    if(system->nextId == 1)
    {
        printf("There are no allocations to resize.\n");
        return;
    }
    allocationId=getNumber(1,system->nextId-1,"Allocation ID: ");
    pos=searchAllocation(system,allocationId);

    if(pos==-1){
        showInvalidId(system,allocationId);
        return;
    }

    updatedSize=getNumber(1,system->totalSize,"New length: ");

    if(updatedSize==system->segments[pos].length)
    {
        printf("Allocation already has that length.\n");
        return;
    }
    if(updatedSize<system->segments[pos].length)
    {
        done=reduceAllocation(system,pos,updatedSize);
    }
    else
    {
        done=expandAllocation(system,pos,updatedSize);
    }
    if(done==-1)
    {
        printf("Resize failed: metadata system allocation failed. Original allocation is unchanged.\n");
        return;
    }
    if(done==0)
    {
        printf("Resize failed. Original allocation is unchanged.\n");
        checkFragmentation(system,updatedSize);
        return;
    }
    pos=searchAllocation(system,allocationId);
    printf("Resize successful. ID: %d, Start: %d, Size: %d\n",allocationId,system->segments[pos].begin,system->segments[pos].length);
}

void printMemoryLayout(struct MemorySystem *system)
{
    int i;
    printf("Memory Map (%d bytes):\n", system->totalSize);
    for(i=0;i<system->segmentCount;i++){
        if(system->segments[i].state == FREE)
        {
            printf("[Free: %d bytes, Start: %d]\n",system->segments[i].length,system->segments[i].begin);
        }
        else{
            printf("[Allocated #%d: %d bytes, Start: %d]\n",system->segments[i].allocationId,system->segments[i].length,system->segments[i].begin);
        }
    }
}

void printMemoryStats(struct MemorySystem *system)
{
    int i,availableSegments=0,largestSegment=0;
    int availableBytes=calculateFreeSpace(system);

    for(i=0;i<system->segmentCount;i++)
    {
        if(system->segments[i].state == FREE)
        {
            availableSegments++;
            if(system->segments[i].length>largestSegment)
            {
                largestSegment = system->segments[i].length;
            }
        }
    }
    printf("Total system: %d bytes\n", system->totalSize);
    printf("Allocated: %d bytes\n", system->totalSize-availableBytes);
    printf("Free: %d bytes\n", availableBytes);
    printf("Free blocks: %d\n", availableSegments);
    printf("Largest free block: %d bytes\n", largestSegment);
    if(system->fragmentationFlag)
    {
        printf("Note: The request for %d bytes failed even though %d bytes are free, because the free system is fragmented.\n", system->failedSize,availableBytes);
        system->fragmentationFlag=0;
    }
}

int main()
{
    struct MemorySystem system;
    int menuChoice=0,totalSize;

    totalSize=getNumber(MIN_POOL,MAX_POOL,"Enter system pool length: ");

    if(!createManager(&system, totalSize))
    {
        printf("Memory allocation failed.\n");
        return 1;
    }

    setStrategy(&system);
    printMenu();

    while(menuChoice != EXIT)
    {
        printf("\n");
        menuChoice = getNumber(ALLOCATE,EXIT,"Choice: ");
        switch(menuChoice){
            case ALLOCATE:
            requestMemory(&system);
            break;
            case RELEASE:
            freeMemory(&system);
            break;
            case RESIZE:
            changeMemorySize(&system);
            break;
            case MAP:
            printMemoryLayout(&system);
            break;
            case STATS:
            printMemoryStats(&system);
            break;
            case CHANGE:
            setStrategy(&system);
            break;
            case EXIT:
            printf("Exiting.\n");
            break;
        }
    }
    destroyManager(&system);
    return 0;
}