#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#define SUCCESS 0
#define NUMBER_OF_SEMAPHORES 2

sem_t sems[NUMBER_OF_SEMAPHORES];

typedef struct argumentsForFunction {
    const char* text;
    int count;
    int start;
} argumetsForFunctionInThread;

int errorCheck(int code, char* inscription) {
    if (code != SUCCESS) {
        perror(inscription);
        return code;
    }
    return SUCCESS;
}

int destroySems(int number) {
    for (int i = 0; i < number; i++) {
        errno = sem_post(&sems[num]);
        if (errorCheck(errno, "Semaphore post error") != SUCCESS) {
            return errno;
        }
        errno = sem_destroy(&sems[i]);
        if (errorCheck(errno, "Destroying semaphore error") != SUCCESS) {
            return errno;
        }
    }
    return SUCCESS;
}

int initializeSems() {
    for (int i = 0; i < NUMBER_OF_SEMAPHORES; ++i) {
        errno = sem_init(&sems[i], 0, i);
        if (errorCheck(errno, "Sem_init error") != SUCCESS) {
            destroySems(i);
            return errno;
        }
    }
    return SUCCESS;
}
int semaphoreWait(int num) {
    errno = sem_wait(&sems[num]);
    if (errorCheck(errno, "Semaphore wait error") != SUCCESS) {
        return errno;
    }
    return SUCCESS;
}

int semaphorePost(int num) {
    errno = sem_post(&sems[num]);
    if (errorCheck(errno, "Semaphore post error") != SUCCESS) {
        return errno;
    }
    return SUCCESS;
}

void* printTextInThread(void* args) {
    argumetsForFunctionInThread* value = (argumetsForFunctionInThread*)args;
    errno = SUCCESS;
    int this_sem = 0,
        next_sem = 0;

    for (int i = 0; i < value->count; i++) {
        this_sem = (value->start + 1) % NUMBER_OF_SEMAPHORES;
        next_sem = (this_sem + 1) % NUMBER_OF_SEMAPHORES;
        errno = semaphoreWait(this_sem);
        if (errorCheck(errno, "Semaphore wait error") != SUCCESS) {
            return NULL;
        }

        printf("%s %d\n", value->text, i);

        errno = semaphorePost(next_sem);
        if (errorCheck(errno, "Semaphore post error") != SUCCESS) {
            return NULL;
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread;
    argumetsForFunctionInThread newThread = { "Hello, I'm new thread", 10, 1};
    argumetsForFunctionInThread mainThread = { "Hello, I'm main thread", 10, 0};
    errno = initializeSems();
    if (errno != SUCCESS) {
        exit(errno);
    }

    errno = pthread_create(&thread, NULL, printTextInThread, &newThread);
    if (errorCheck(errno, "Creating thread error") != SUCCESS) {
        destroySems(NUMBER_OF_SEMAPHORES);
        exit(errno);
    }

    printTextInThread(&mainThread);

    errno = pthread_join(thread, NULL);
    if (errorCheck(errno, "Joining thread error") != SUCCESS) {
        destroySems(NUMBER_OF_SEMAPHORES);
        exit(errno);
    }

    destroySems(NUMBER_OF_SEMAPHORES);
    return SUCCESS;
}
