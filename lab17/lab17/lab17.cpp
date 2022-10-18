#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define SUCCESS 0

pthread_mutex_t mutex;

typedef struct Node {
    char* text;
    struct Node* next;
} Node;

void printErrorAndTerminateProgram(int valueError, char* msg) {
    fprintf(stderr, "%s cause : %s\n", msg, strerror(valueError));
    exit(EXIT_FAILURE);
}

void destroyMutexes() {
    int code = pthread_mutex_destroy(&mutex);
    if (code != SUCCESS) {
        printErrorAndTerminateProgram(code, "mutex could not  be destroy");
    }
}

void lockMutex() {
    int code = pthread_mutex_lock(&mutex);
    if (code != SUCCESS) {
        destroyMutexes();
        printErrorAndTerminateProgram(code, "mutex could not do lock");
    }
}

void unlockMutex() {
    int code = pthread_mutex_unlock(&mutex);
    if (code != SUCCESS) {
        destroyMutexes();
        printErrorAndTerminateProgram(code, "mutex could not do unlock");
    }
}

void push(Node** tmp, char* text) {
    lockMutex();
    Node* newList = (Node*)malloc(sizeof(Node));
    newList->text = (char*)malloc(sizeof(char) * strlen(text));
    for (int i = 0; i < strlen(text); ++i) {
        newList->text[i] = text[i];
    }
    newList->next = *tmp;
    *tmp = newList;
    unlockMutex();
}

void printList(Node* head) {
    lockMutex();
    while (head) {
        printf("%s", head->text);
        head = head->next;
    }
    printf("\n");
    unlockMutex();
}

void enterLines(char* value) {
    printf("Please, enter the line\n");
    if (fgets(value, 80, stdin) == NULL) {
        exit(0);
    }
}

void initializeMutexes() {
    pthread_mutexattr_t attr;
    int code = pthread_mutexattr_init(&attr);
    if (code != SUCCESS) {
        printErrorAndTerminateProgram(code, "Mutex attributes could not be created");
    }

    code = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (code != SUCCESS) {
        printErrorAndTerminateProgram(code, "Mutex attribute type could not be set");
    }

    code = pthread_mutex_init(&mutex, &attr);
    if (code != SUCCESS) {
        destroyMutexes();
        printErrorAndTerminateProgram(code, "Mutex init error");
    }
}

void sortList(Node* head) {
    lockMutex();
    for (Node* p = head; p != NULL; p = p->next) {
        for (Node* tmp = p; tmp->next != NULL; tmp = tmp->next) {
            if (strcmp(tmp->text, tmp->next->text) > 0) {
                char* value = tmp->text;
                tmp->text = tmp->next->text;
                tmp->next->text = value;
            }
        }
    }
    unlockMutex();
}

void* waitSort(void* head) {
    Node** value = (Node**)head;
    while (1) {
        sleep(5);
        sortList(*value);
    }
    return NULL;
}

int main(int argc, char** argv) {
    Node* head = NULL;
    char value[80];
    initializeMutexes();
    pthread_t ntid;
    int err = pthread_create(&ntid, NULL, waitSort, (void*)&head);
    if (err != SUCCESS) {
        destroyMutexes();
        printErrorAndTerminateProgram(err, "unable to create thread");
    }
    while (true)
    {
        enterLines(value);
        if (value[0] == '\n')
            printList(head);
        else {
            push(&head, value);
        }
    }

    return EXIT_SUCCESS;
}
