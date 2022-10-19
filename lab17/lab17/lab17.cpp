#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define SUCCESS 0
#define FAILURE 1

pthread_mutex_t mutex;
bool flag = true;

typedef struct Node {
    char* text;
    struct Node* next;
} Node;

enum listOperations {
    outputList,
    pushingInList,
    stopWorkingList
};

void deleteList(Node** head) {
    Node* prev = NULL;
    if ((*head) != NULL) {
        while ((*head)->next) {
            prev = (*head);
            (*head) = (*head)->next;
            free(prev->text);
            free(prev);
        }
        free((*head)->text);
        free(*head);
    }
}

void printError(int valueError, char* msg) {
    char buf[1024];
    strerror_r(valueError, buf, sizeof(buf));
    fprintf(stderr, "%s cause : %s\n", msg, buf);
}

int destroyMutexes() {
    int code = pthread_mutex_destroy(&mutex);
    if (code != SUCCESS) {
        printError(code, "Mutex could not  be destroy");
        return FAILURE;
    }
    return SUCCESS;
}

int lockMutex() {
    int code = pthread_mutex_lock(&mutex);
    if (code != SUCCESS) {
        printError(code, "Mutex could not do lock");
        return FAILURE;
    }
    return SUCCESS;
}

int unlockMutex() {
    int code = pthread_mutex_unlock(&mutex);
    if (code != SUCCESS) {
        printError(code, "Mutex could not do unlock");
        return FAILURE;
    }
    return SUCCESS;
}

int push(Node** tmp, char* text) {
    int code = lockMutex();
    if (code != SUCCESS) {
        destroyMutexes();
        deleteList(tmp);
        exit(EXIT_FAILURE);
    }
    Node* newList = (Node*)malloc(sizeof(Node));
    newList->text = (char*)malloc(sizeof(char) * strlen(text));
    for (int i = 0; i < strlen(text); ++i) {
        newList->text[i] = text[i];
    }
    newList->next = *tmp;
    *tmp = newList;
    code = unlockMutex();
    if (code != SUCCESS) {
        destroyMutexes();
        deleteList(tmp);
        exit(EXIT_FAILURE);
    }
}

void printList(Node** head) {
    int code = lockMutex();
    if (code != SUCCESS) {
        destroyMutexes();
        deleteList(head);
        exit(EXIT_FAILURE);
    }
    Node* value = *head;
    while (value) {
        printf("%s\n", (value)->text);
        (value) = (value)->next;
    }
    printf("\n");
    code = unlockMutex();
    if (code != SUCCESS) {
        destroyMutexes();
        deleteList(head);
        exit(EXIT_FAILURE);
    }
}

int enterLines(char* value) {
    printf("Please, enter the line! To stop the program, enter 'end'. Press 'Enter' to print the list.\n");
    if (fgets(value, 80, stdin) == NULL) {
        return FAILURE;
    }
    if (value[0] != '\n') {
        if (strchr(value, '\n') != NULL) {
            *strchr(value, '\n') = '\0';
        }
    }
    return SUCCESS;
}

void initializeMutexes() {
    pthread_mutexattr_t attr;
    int code = pthread_mutexattr_init(&attr);
    if (code != SUCCESS) {
        printError(code, "Mutex attributes could not be created");
        exit(EXIT_FAILURE);
    }

    code = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (code != SUCCESS) {
        printError(code, "Mutex attribute type could not be set");
        exit(EXIT_FAILURE);
    }

    code = pthread_mutex_init(&mutex, &attr);
    if (code != SUCCESS) {
        printError(code, "Mutex init error");
        exit(EXIT_FAILURE);
    }
}

void sortList(Node** head) {
    for (Node* p = *head; p != NULL; p = p->next) {
        for (Node* tmp = p; tmp->next != NULL; tmp = tmp->next) {
            if (strcmp(tmp->text, tmp->next->text) > 0) {
                char* value = tmp->text;
                tmp->text = tmp->next->text;
                tmp->next->text = value;
            }
        }
    }
}

void* waitSort(void* head) {
    Node** value = (Node**)head;
    while (flag) {
        sleep(5);
        int code = lockMutex();
        if (code != SUCCESS) {
            destroyMutexes();
            deleteList(value);
            exit(EXIT_FAILURE);
        }
        sortList(value);
        code = unlockMutex();
        if (code != SUCCESS) {
            destroyMutexes();
            deleteList(value);
            exit(EXIT_FAILURE);
        }
    }
    return NULL;
}

int checkOperations(char* value) {
    if (value[0] == '\n') {
        return outputList;
    }
    if (strcmp(value, "end") == 0) {
        return stopWorkingList;
    }
    return pushingInList;
}

void doOperationWithList(char* value, Node** head) {
    while (flag == true) {
        int code = enterLines(value);
        if (code != SUCCESS) {
            destroyMutexes();
            deleteList(head);
            exit(EXIT_FAILURE);
        }
        switch (checkOperations(value)) {
            case outputList:
                printList(head);
                break;
            case pushingInList:
                push(head, value);
                break;
            case stopWorkingList:
                flag = false;
                break;
        }
    }
}

int main(int argc, char** argv) {
    Node* head = NULL;
    char value[80];
    initializeMutexes();
    pthread_t ntid;
    int err = pthread_create(&ntid, NULL, waitSort, (void*)&head);
    if (err != SUCCESS) {
        destroyMutexes();
        printError(err, "Unable to create thread");
        exit(EXIT_FAILURE);
    }

    doOperationWithList(value, &head);

    err = pthread_join(ntid, NULL);
    if (err != SUCCESS) {
        deleteList(&head);
        destroyMutexes();
        printError(err, "Error in the join function");
        exit(EXIT_FAILURE);
    }

    destroyMutexes();
    deleteList(&head);
    return EXIT_SUCCESS;
}
