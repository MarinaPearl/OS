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
#define FAILURE 1

typedef struct Node {
    char* text;
    struct Node* next;
} Node;

void cleanResources(Node** head);
int push(Node** head, char* text);
int printList(Node** head);
int initializeResources(Node** head);
int sortList(Node** head);

#endif // LAB17_H