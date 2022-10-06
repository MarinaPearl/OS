#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <string.h>
#include <stdbool.h>

#define SUCCESS 0
#define COUNT_MUTEXES 3

pthread_mutex_t arrayMutex[COUNT_MUTEXES];
bool ready = false;

typedef struct argumentsForFunction {
    const char* text;
    int count;
} argumetsForFunctionInThread;

void printErrorAndTerminateProgram(int valueError, char* msg) {
    fprintf(stderr, "%s cause : %s\n", msg, strerror(valueError));
    exit(EXIT_FAILURE);
}

void destroyMutexes(int index) {
    for (int i = 0; i < index; ++i) {
        int code = pthread_mutex_destroy(&arrayMutex[i]);
        if (code != SUCCESS) {
            printErrorAndTerminateProgram(code, "mutex could not  be destroy");
        }
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

    for (int i = 0; i < COUNT_MUTEXES; ++i) {
        code = pthread_mutex_init(&arrayMutex[i], &attr);
        if (code != SUCCESS) {
            destroyMutexes(i);
            printErrorAndTerminateProgram(code, "Mutex init error");
        }
    }
}

void lockMutex(int i) {
    int code = pthread_mutex_lock(&arrayMutex[i]);
    if (code != SUCCESS) {
        destroyMutexes(COUNT_MUTEXES);
        printErrorAndTerminateProgram(code, "mutex could not do lock");
    }
}

void unlockMutex(int i) {
    int code = pthread_mutex_unlock(&arrayMutex[i]);
    if (code != SUCCESS) {
        destroyMutexes(COUNT_MUTEXES);
        printErrorAndTerminateProgram(code, "mutex could not do unlock");
    }
}

void* printTextInThread(void* args) {
    argumetsForFunctionInThread* value = (argumetsForFunctionInThread*)args;
    int thisMutex = 0;
    int nextMutex = 0;
    if (!ready) {
        thisMutex = 2;
        lockMutex(thisMutex);
        ready = true;
    }
    for (int i = 0; i < value->count; ++i) {
        nextMutex = (thisMutex + 1) % COUNT_MUTEXES;
        lockMutex(nextMutex);
        printf("%d %s\n", i, value->text);
        unlockMutex(thisMutex);
        thisMutex = nextMutex;
    }
    unlockMutex(thisMutex);
    return NULL;
}

void* printMain(void* args) {
    argumetsForFunctionInThread* value = (argumetsForFunctionInThread*)args;
    printf("%s\n", value->text);
    unlockMutex(0);
    lockMutex(1);
    printf("%s\n", value->text);
    unlockMutex(1);
    return NULL;
}

void* printChild(void* args) {
    argumetsForFunctionInThread* value = (argumetsForFunctionInThread*)args;
    lockMutex(1);
    ready = true;
    lockMutex(0);
    printf("%s\n", value->text);
    unlockMutex(0);
    unlockMutex(1);
    lockMutex(1);
    printf("%s\n", value->text);
    unlockMutex(1);
    return NULL;
}

int main() {
    initializeMutexes();
    pthread_t ntid;
    argumetsForFunctionInThread newThread = { "Hello, I'm new thread\n", 10 };
    argumetsForFunctionInThread mainThread = { "Hello, I'm main thread\n", 10 };
    lockMutex(0);

    int err = pthread_create(&ntid, NULL, printChild, (void*)&newThread);
    if (err != SUCCESS) {
        unlockMutex(0);
        destroyMutexes(COUNT_MUTEXES);
        printErrorAndTerminateProgram(err, "unable to create thread");
    }
    while (ready != true) {};

    printMain((void*)&mainThread);

    err = pthread_join(ntid, NULL);
    if (err != SUCCESS) {
        destroyMutexes(COUNT_MUTEXES);
        printErrorAndTerminateProgram(err, "error in the join function");
    }

    destroyMutexes(COUNT_MUTEXES);
    return EXIT_SUCCESS;
}