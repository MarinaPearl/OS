#include "lab17.h"

#define END_OF_READING 2
#define MAX_LENGTH_LINE 80
#define MIN_LENGTH_LINE 1


bool LIST_WORK = true;
bool STOP = true;

enum listOperations {
    LIST_OPERATIONS_OUTPUT,
    LIST_OPERATIONS_PUSHING,
    LIST_OPERATIONS_STOP_WORKING,
    LIST_OPERATIONS_IGNORING_SYMBOL
};

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

int findOperations(char* value, int code) {
    if (code == END_OF_READING) {
        return LIST_OPERATIONS_STOP_WORKING;
    }

    if (value[0] == '\n' && strlen(value) == MIN_LENGTH_LINE) {
        return LIST_OPERATIONS_OUTPUT;
    }
    
    if (strcmp(value, "end") == 0) {
        return LIST_OPERATIONS_STOP_WORKING;
    }
    
    if (value[0] == '\0') {
        return LIST_OPERATIONS_IGNORING_SYMBOL;
    }
    return LIST_OPERATIONS_PUSHING;
}

int doOperationWithList(Node** head) {
    int code = SUCCESS;
    char value[MAX_LENGTH_LINE + 1];
    while (LIST_WORK == true) {
        int code = enterLines(value);
        if (code == FAILURE) {
            perror("An error occurred while reading the input stream");
            cleanResources(head);
            exit(EXIT_FAILURE);
        }
        switch (findOperations(value, code)) {
            case LIST_OPERATIONS_OUTPUT:
                code = printList(head);
                break;
            case LIST_OPERATIONS_PUSHING:
                code = push(head, value);
                break;
            case LIST_OPERATIONS_STOP_WORKING:
                LIST_WORK = false;
                break;
            case LIST_OPERATIONS_IGNORING_SYMBOL:
                break;
        }
        if (code != SUCCESS) {
            LIST_WORK = false;
        }
    }
    return code;
}

void* waitSort(void* head) {
    Node** value = (Node**)head;
    int code = SUCCESS;
    while (LIST_WORK == true) {
        sleep(5);
        code = sortList(value);
        if (code != SUCCESS) {
            LIST_WORK = false;
        }
    }
    pthread_exit((void*)&code);
}

int main(int argc, char** argv) {
    Node* head;
    pthread_t ntid;
    initializeResources(&head);

    errno = pthread_create(&ntid, NULL, waitSort, (void*)&head);
    if (errno != SUCCESS) {
        perror("Unable to create thread");
        cleanResources(&head);
        exit(EXIT_FAILURE);
    }
    printf("To add to the list: enter a string.\nTo display the list : press 'enter'.\nTo end the program : enter 'end'.\n");
    int code = doOperationWithList(&head);
    if (code != SUCCESS) {
        exit(EXIT_FAILURE);
    }
    void* returnValue;
    errno = pthread_join(ntid, &returnValue);
    if (errno != SUCCESS) {
        perror("Error in the join function");
        cleanResources(&head);
        exit(EXIT_FAILURE);
    }
    int code = *(int*)returnValue;
    if (code != SUCCESS) {
        cleanResources(&head);
        exit(EXIT_FAILURE);
    }
    cleanResources(&head);
    return EXIT_SUCCESS;
}