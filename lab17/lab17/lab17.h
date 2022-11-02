#ifndef LAB17_H
#define LAB17_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define SUCCESS 0

typedef struct Node {
    char* text;
    struct Node* next;
} Node;

void cleanResources(Node** head);
void push(Node** head, char* text);
void printList(Node** head);
void initializeResources(Node** head);
void sortList(Node** head);

#endif // LAB17_H