#include "lab17.h"

pthread_mutex_t MUTEX;

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

void printError(char* msg) {
    perror(msg);
}

int destroyMutexes() {
    errno = pthread_mutex_destroy(&MUTEX);
    if (errno != SUCCESS) {
        printError("Mutex could not  be destroy");
    }
    return errno;
}

void cleanResources(Node** head) {
    destroyMutexes();
    deleteList(head);
}

int lockMutex() {
    errno = pthread_mutex_lock(&MUTEX);
    if (errno != SUCCESS) {
        printError("Mutex could not do lock");
    }
    return errno;
}

int unlockMutex() {
    errno = pthread_mutex_unlock(&MUTEX);
    if (errno != SUCCESS) {
        printError("Mutex could not do unlock");
    }
    return errno;
}

void push(Node** head, char* text) {
    errno = lockMutex();
    if (errno != SUCCESS) {
        cleanResources(head);
        exit(EXIT_FAILURE);
    }

    Node* newList = (Node*)malloc(sizeof(Node));
    if (newList == NULL) {
        printError("Error in malloc");
        cleanResources(head);
        exit(EXIT_FAILURE);
    }
    newList->text = (char*)malloc(sizeof(char) * strlen(text));
    if (newList->text == NULL) {
        printError("Error in malloc");
        cleanResources(head);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < strlen(text); ++i) {
        newList->text[i] = text[i];
    }
    newList->next = *head;
    *head = newList;

    errno = unlockMutex();
    if (errno != SUCCESS) {
        cleanResources(head);
        exit(EXIT_FAILURE);
    }
}

void printList(Node** head) {
    errno = lockMutex();
    if (errno != SUCCESS) {
        cleanResources(head);
        exit(EXIT_FAILURE);
    }
    Node* value = *head;
    printf("List:\n");
    while (value) {
        printf("%s\n", (value)->text);
        (value) = (value)->next;
    }
    printf("\n");
    errno = unlockMutex();
    if (errno != SUCCESS) {
        cleanResources(head);
        exit(EXIT_FAILURE);
    }
}

int enterLines(char* value) {
    if (fgets(value, MAX_LENGTH_LINE + 1, stdin) == NULL) {
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
    if (strlen(value) == MAX_LENGTH_LINE) {
        STOP = false;
    }
    if (value[0] != '\n' && (strchr(value, '\n') != NULL)) {
        *strchr(value, '\n') = '\0';
    }
    return SUCCESS;
}

void initializeMutexes() {
    pthread_mutexattr_t attr;
    errno = pthread_mutexattr_init(&attr);
    if (errno != SUCCESS) {
        printError("Mutex attributes could not be created");
        exit(EXIT_FAILURE);
    }

    errno = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (errno != SUCCESS) {
        printError("Mutex attribute type could not be set");
        exit(EXIT_FAILURE);
    }

    errno = pthread_mutex_init(&MUTEX, &attr);
    if (errno != SUCCESS) {
        printError("Mutex init error");
        exit(EXIT_FAILURE);
    }
}

void sortList(Node** head) {
    errno = lockMutex();
    if (errno != SUCCESS) {
        cleanResources(head);
        exit(EXIT_FAILURE);
    }
    for (Node* p = *head; p != NULL; p = p->next) {
        for (Node* tmp = p->next; tmp != NULL; tmp = tmp->next) {
            if (strcmp(p->text, tmp->text) > 0) {
                char* value = tmp->text;
                tmp->text = p->text;
                p->text = value;
            }
        }
    }
    errno = unlockMutex();
    if (errno != SUCCESS) {
        cleanResources(head);
        exit(EXIT_FAILURE);
    }
}

void* waitSort(void* head) {
    Node** value = (Node**)head;
    while (LIST_WORK == true) {
        sleep(5);
        sortList(value);
    }
    pthread_exit(NULL);
}

int findOperations(char* value, int code) {
    if (code == END_OF_READING) {
        return stopWorkingList;
    }
    if (value[0] == '\n' && strlen(value) == MIN_LENGTH_LINE) {
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

void initializeResources() {
    initializeMutexes();
}

void doOperationWithList(Node** head) {
    char value[MAX_LENGTH_LINE + 1];
    while (LIST_WORK == true) {
        int code = enterLines(value);
        if (code == FAILURE) {
            printError("An error occurred while reading the input stream");
            cleanResources(head);
            exit(EXIT_FAILURE);
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