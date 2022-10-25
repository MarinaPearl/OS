#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#define SUCCESS 0
#define FAILURE 1
#define END_OF_READING 2

pthread_mutex_t MUTEX;
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
    int code = pthread_mutex_destroy(&MUTEX);
    if (code != SUCCESS) {
        printError(code, "Mutex could not  be destroy");
        return FAILURE;
    }
    return SUCCESS;
}

void cleanResourcesAndAbortProgram(Node** head) {
    destroyMutexes();
    deleteList(head);
    exit(EXIT_FAILURE);
}

int lockMutex() {
    int code = pthread_mutex_lock(&MUTEX);
    if (code != SUCCESS) {
        printError(code, "Mutex could not do lock");
        return FAILURE;
    }
    return SUCCESS;
}

int unlockMutex() {
    int code = pthread_mutex_unlock(&MUTEX);
    if (code != SUCCESS) {
        printError(code, "Mutex could not do unlock");
        return FAILURE;
    }
    return SUCCESS;
}

int push(Node** head, char* text) {
    int code = lockMutex();
    if (code != SUCCESS) {
        cleanResourcesAndAbortProgram(head);
    }
    Node* newList = (Node*)malloc(sizeof(Node));
    newList->text = (char*)malloc(sizeof(char) * strlen(text));
    for (int i = 0; i < strlen(text); ++i) {
        newList->text[i] = text[i];
    }
    newList->next = *head;
    *head = newList;
    code = unlockMutex();
    if (code != SUCCESS) {
        cleanResourcesAndAbortProgram(head);
    }
}

void printList(Node** head) {
    int code = lockMutex();
    if (code != SUCCESS) {
        cleanResourcesAndAbortProgram(head);
    }
    Node* value = *head;
    printf("List:\n");
    while (value) {
        printf("%s\n", (value)->text);
        (value) = (value)->next;
    }
    printf("\n");
    code = unlockMutex();
    if (code != SUCCESS) {
        cleanResourcesAndAbortProgram(head);
    }
}

int enterLines(char* value) {
    if (fgets(value, 81, stdin) == NULL) {
        if (ferror(stdin) != SUCCESS) {
            return FAILURE;
        }
        return END_OF_READING;
    }
    if (STOP == false) {
        if (value[0] == '\n') {
            value[0] = '\0';
        }
        STOP = true;
    }
    if (strlen(value) == 80) {
        STOP = false;
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

    code = pthread_mutex_init(&MUTEX, &attr);
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
    while (LIST_WORK == true) {
        sleep(5);
        int code = lockMutex();
        if (code != SUCCESS) {
            cleanResourcesAndAbortProgram(value);
        }
        sortList(value);
        code = unlockMutex();
        if (code != SUCCESS) {
            cleanResourcesAndAbortProgram(value);
        }
    }
    return NULL;
}

int findOperations(char* value, int code) {
    if (code == END_OF_READING) {
        return stopWorkingList;
    }
    if (value[0] == '\n' && strlen(value) == 1) {
        return outputList;
    }
    if (strcmp(value, "end") == 0) {
        return stopWorkingList;
    }
    if (value[0] == '\0') {
        return ignoringSymbol;
    }
    return pushingInList;
}

void doOperationWithList(Node** head) {
    char value[81];
    while (LIST_WORK == true) {
        int code = enterLines(value);
        if (code == FAILURE) {
            printError(code, "An error occurred while reading the input stream");
            cleanResourcesAndAbortProgram(head);
        }
        switch (findOperations(value, code)) {
            case outputList:
                printList(head);
                break;
            case pushingInList:
                push(head, value);
                break;
            case stopWorkingList:
                LIST_WORK = false;
                break;
            case ignoringSymbol:
                break;
        }
    }
}

int main(int argc, char** argv) {
    Node* head = NULL;
    pthread_t ntid;
    initializeMutexes();
    int err = pthread_create(&ntid, NULL, waitSort, (void*)&head);
    if (err != SUCCESS) {
        destroyMutexes();
        printError(err, "Unable to create thread");
        exit(EXIT_FAILURE);
    }

    printf("To add to the list: enter a string.\nTo display the list : press 'enter'.\nTo end the program : enter 'end'.\n");
    doOperationWithList(&head);

    err = pthread_join(ntid, NULL);
    if (err != SUCCESS) {
        printError(err, "Error in the join function");
        cleanResourcesAndAbortProgram(&head);
    }

    destroyMutexes();
    deleteList(&head);
    return EXIT_SUCCESS;
}
