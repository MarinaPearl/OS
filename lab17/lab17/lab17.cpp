#include "lab17.h"

int main(int argc, char** argv) {
    Node* head = NULL;
    pthread_t ntid;
    initializeResources();
    errno = pthread_create(&ntid, NULL, waitSort, (void*)&head);
    if (errno != SUCCESS) {
        printError("Unable to create thread");
        cleanResources(&head);
        exit(EXIT_FAILURE);
    }

    printf("To add to the list: enter a string.\nTo display the list : press 'enter'.\nTo end the program : enter 'end'.\n");
    doOperationWithList(&head);

    errno = pthread_join(ntid, NULL);
    if (errno != SUCCESS) {
        printError("Error in the join function");
        cleanResources(&head);
        exit(EXIT_FAILURE);
    }

    cleanResources(&head);
    return EXIT_SUCCESS;
}