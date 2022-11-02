#include "lab17.h"

bool LIST_WORK = true;
bool STOP = true;

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

void doOperationWithList(Node** head) {
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
                printList(head);
                break;
            case LIST_OPERATIONS_PUSHING:
                push(head, value);
                break;
            case LIST_OPERATIONS_STOP_WORKING:
                LIST_WORK = false;
                break;
            case LIST_OPERATIONS_IGNORING_SYMBOL:
                break;
        }
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

int main(int argc, char** argv) {
    Node* head = NULL;
    pthread_t ntid;
    initializeResources();

    errno = pthread_create(&ntid, NULL, waitSort, (void*)&head);
    if (errno != SUCCESS) {
        perror("Unable to create thread");
        cleanResources(&head);
        exit(EXIT_FAILURE);
    }

    printf("To add to the list: enter a string.\nTo display the list : press 'enter'.\nTo end the program : enter 'end'.\n");
    doOperationWithList(&head);

    errno = pthread_join(ntid, NULL);
    if (errno != SUCCESS) {
        perror("Error in the join function");
        cleanResources(&head);
        exit(EXIT_FAILURE);
    }

    cleanResources(&head);
    return EXIT_SUCCESS;
}