#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <string.h>
#include <errno.h>

#define SUCCESS 0
#define FAILURE 1
#define COUNT_ITERATIONS 10
#define NUMBER_THREAD_MAIN 0
#define NUMBER_THREAD_NEW 1
#define COUNT_THREADS 2

pthread_mutex_t mutex;
pthread_cond_t condition;
int flag = 0;
int codeReturn = SUCCESS;

typedef struct argumentsForFunction {
    const char* text;
    int countIterations;
    int numberThread;
} argumetsForFunctionInThread;

void destroyMutex() {
    errno = pthread_mutex_destroy(&mutex);
    if (errno != SUCCESS) {
        perror("Mutex could not  be destroy");
    }
}

void destroyCondition() {
    errno = pthread_cond_destroy(&condition);
    if (errno != SUCCESS) {
        perror("Condition variable ould not  be destroy");
    }
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

int lockMutex() {
    errno = pthread_mutex_lock(&mutex);
    if (errno != SUCCESS) {
        perror("Mutex could not do lock");
        return FAILURE;
    }
    return SUCCESS;
}

int unlockMutex() {
    errno = pthread_mutex_unlock(&mutex);
    if (errno != SUCCESS) {
        perror("Mutex could not do unlock");
        return FAILURE;
    }
    return SUCCESS;
}

int initializeResourses() {
    int code = initializeMutexes();
    if (code != SUCCESS) {
        return FAILURE;
    }
    errno = pthread_cond_init(&condition, NULL);
    if (errno != SUCCESS) {
        destroyMutex();
        perror("Error in condition init");
        return FAILURE;
    }
    return  SUCCESS;
}

int waitCondition() {
    errno = pthread_cond_wait(&condition, &mutex);
    if (errno != SUCCESS) {
        perror("Error in condition wait");
        return FAILURE;
    }
    return SUCCESS;
}

int sendSignalCondition() {
    errno = pthread_cond_signal(&condition);
    if (errno != SUCCESS) {
        perror("Error in condition");
        return FAILURE;
    }
    return SUCCESS;
}

void* printTextInThread(void* args) {
    argumetsForFunctionInThread* value = (argumetsForFunctionInThread*)args;
    for (int i = 0; i < value->countIterations; ++i) {
        codeReturn = lockMutex();
        if (codeReturn != SUCCESS) {
            return (void*)&codeReturn;
        }
        while (flag != value->numberThread) {
            codeReturn = waitCondition();
            if (codeReturn != SUCCESS) {
                return (void*)&codeReturn;
            }
        }
        printf("%d %s\n", i, value->text);
        flag = (value->numberThread + 1) % COUNT_THREADS;
        codeReturn = sendSignalCondition();
        if (codeReturn != SUCCESS) {
            return (void*)&codeReturn;
        }
        codeReturn = unlockMutex();
        if (codeReturn != SUCCESS) {
            return (void*)&codeReturn;
        }
    }
    return (void*)&codeReturn;
}

void freeResourses() {
    destroyCondition();
    destroyMutex();
}

int main() {
    pthread_t ntid;
    argumetsForFunctionInThread newThread = { "Hello, I'm new thread\n", COUNT_ITERATIONS, NUMBER_THREAD_NEW};
    argumetsForFunctionInThread mainThread = { "Hello, I'm main thread\n", COUNT_ITERATIONS, NUMBER_THREAD_MAIN};
    int code = initializeResourses();
    if (code != SUCCESS) {
        exit(EXIT_FAILURE);
    }
    errno = pthread_create(&ntid, NULL, printTextInThread, (void*)&newThread);
    if (errno != SUCCESS) {
        perror("Error in pthread create");
        freeResourses();
        exit(EXIT_FAILURE);
    }
    void* returnValue = printTextInThread((void*)&mainThread);
    int err = *(int*)returnValue;
    if (err != SUCCESS) {
        freeResourses();
        exit(EXIT_FAILURE);
    }
    errno = pthread_join(ntid, &returnValue);
    if (errno != SUCCESS) {
        perror("Error in the join function");
        freeResourses();
        exit(EXIT_FAILURE);
    }
    err = *(int*)returnValue;
    if (err != SUCCESS) {
        freeResourses();
        exit(EXIT_FAILURE);
    }
    freeResourses();
    return EXIT_SUCCESS;
}