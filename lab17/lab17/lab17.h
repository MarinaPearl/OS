#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define SUCCESS 0
#define FAILURE 1
#define END_OF_READING 2
#define MAX_LENGTH_LINE 80
#define MIN_LENGTH_LINE 1

bool LIST_WORK = true;
bool STOP = true;

typedef struct Node {
    char* text;
    struct Node* next;
} Node;

enum listOperations {
    outputList,
    pushingInList,
    stopWorkingList,
    ignoringSymbol
};

void deleteList(Node** head);
void printError(char* msg);
int destroyMutexes();
void cleanResources(Node** head);
int lockMutex();
int unlockMutex();
void push(Node** head, char* text);
void printList(Node** head);
int enterLines(char* value);
void initializeMutexes();
void* waitSort(void* head);
int findOperations(char* value, int code);
void doOperationWithList(Node** head);
void initializeResources();