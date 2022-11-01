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

void cleanResources(Node** head);
void push(Node** head, char* text);
void printList(Node** head);
void initializeMutexes();
void initializeResources();
void sortList(Node** head);