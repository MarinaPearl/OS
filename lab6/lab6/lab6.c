#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define MAX_LENGTH_LINE 100
#define MAX_QUANTITY_LINES 100
#define SUCCESS 0
#define FAILURE 1
#define END_OF_READING 2
#define COEFFICIENT 190000

int countThread = 0;

typedef struct argumentsForFunction {
    char text[MAX_LENGTH_LINE];
    size_t lenght;
} argumetsForFunctionInThread;

void* printLine(void* args) {
    argumetsForFunctionInThread* value = (argumetsForFunctionInThread*)args;
    usleep(COEFFICIENT * value->lenght);
    printf("%s\n", value->text);
    pthread_exit(NULL);
}

int readLines(argumetsForFunctionInThread* array) {
    printf("Enter lines, please! Or enter 'end' for completions programm!\nMaximum number of rows = 100\n");
    while (fgets(array[countThread].text, MAX_LENGTH_LINE - 1, stdin) != NULL) {
        if (strcmp(array[countThread].text, "end\n") == 0 || countThread == MAX_QUANTITY_LINES) {
            break;
        }
        if (strchr(array[countThread].text, '\n') != NULL) {
            *strchr(array[countThread].text, '\n') = '\0';
        }
        array[countThread].lenght = strlen(array[countThread].text);
        ++countThread;
    }
    if (ferror(stdin) != SUCCESS) {
        return FAILURE;
    }
    return SUCCESS;
}

int main() {
    argumetsForFunctionInThread args[MAX_QUANTITY_LINES];
    pthread_t ntid[MAX_QUANTITY_LINES];
    int code = readLines(args);
    if (code != SUCCESS) {
        perror("Error in read lines");
        exit(EXIT_FAILURE);
    }
    printf("\nList:\n");
    for (int i = 0; i < countThread; ++i) {
        errno = pthread_create(&ntid[i], NULL, printLine, (void*)&args[i]);
        if (errno != SUCCESS) {
            perror("Error in create thread");
            exit(EXIT_FAILURE);
        }
    }
    for (int i = 0; i < countThread; ++i) {
        errno = pthread_join(ntid[i], NULL);
        if (errno != SUCCESS) {
            perror("Error in join");
            exit(EXIT_FAILURE);
        }
    }
    return EXIT_SUCCESS;
}