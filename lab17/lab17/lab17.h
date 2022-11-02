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

typedef struct Node {
    char* text;
    struct Node* next;
} Node;

enum listOperations {
    LIST_OPERATIONS_OUTPUT,
    LIST_OPERATIONS_PUSHING,
    LIST_OPERATIONS_STOP_WORKING,
    LIST_OPERATIONS_IGNORING_SYMBOL
};

void cleanResources(Node** head);
void push(Node** head, char* text);
void printList(Node** head);
void initializeResources(Node** head);
void sortList(Node** head);