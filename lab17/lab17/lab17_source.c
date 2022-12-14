#include "lab17.h"

pthread_mutex_t mutex;

void deleteList(Node** head) {
    Node* prev = NULL;
    if ((*head) != NULL) {
        while ((*head)->next != NULL) {
            prev = (*head);
            (*head) = (*head)->next;
            free(prev->text);
            free(prev);
        }
        free((*head)->text);
        free(*head);
    }
}

void destroyMutex() {
    errno = pthread_mutex_destroy(&mutex);
    if (errno != SUCCESS) {
        perror("Mutex could not  be destroy");
    }
}

void cleanResources(Node** head) {
    destroyMutex();
    deleteList(head);
}

int lockMutex() {
    errno = pthread_mutex_lock(&mutex);
    if (errno != SUCCESS) {
        perror("Mutex could not do lock");
    }
    return errno;
}

int unlockMutex() {
    errno = pthread_mutex_unlock(&mutex);
    if (errno != SUCCESS) {
        perror("Mutex could not do unlock");
    }
    return errno;
}

int push(Node** head, char* text) {
    errno = lockMutex();
    if (errno != SUCCESS) {
        return FAILURE;
    }

    Node* newList = (Node*)malloc(sizeof(Node));
    if (newList == NULL) {
        perror("Error in malloc");
        return FAILURE;
    }

    newList->text = (char*)malloc(sizeof(char) * strlen(text));
    if (newList->text == NULL) {
        perror("Error in malloc");
        return FAILURE;
    }

    for (int i = 0; i < strlen(text); ++i) {
        newList->text[i] = text[i];
    }
    newList->next = *head;
    *head = newList;

    errno = unlockMutex();
    if (errno != SUCCESS) {
        return FAILURE;
    }
    return SUCCESS;
}

int printList(Node** head) {
    errno = lockMutex();
    if (errno != SUCCESS) {
        return FAILURE;
    }

    Node* value = *head;
    printf("List:\n");
    while (value != NULL) {
        printf("%s\n", value->text);
        value = value->next;
    }

    printf("\n");
    errno = unlockMutex();
    if (errno != SUCCESS) {
        return FAILURE;
    }
    return SUCCESS;
}

int initializeMutexes() {
    pthread_mutexattr_t attr;
    errno = pthread_mutexattr_init(&attr);
    if (errno != SUCCESS) {
        perror("Mutex attributes could not be created");
        return FAILURE;
    }

    errno = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (errno != SUCCESS) {
        perror("Mutex attribute type could not be set");
        return FAILURE;
    }

    errno = pthread_mutex_init(&mutex, &attr);
    if (errno != SUCCESS) {
        perror("Mutex init error");
        return FAILURE;
    }
    return SUCCESS;
}

int sortList(Node** head) {
    errno = lockMutex();
    if (errno != SUCCESS) {
        return FAILURE;
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
        return FAILURE;
    }
    return SUCCESS;
}

int initializeResources(Node** head) {
    *head = NULL;
    int code = initializeMutexes();
    return code;
}