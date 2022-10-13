#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>

#define SUCCESS 0
#define NUMBER_OF_SEMAPHORES 2

sem_t sems[NUMBER_OF_SEMAPHORES];

typedef struct argumentsForFunction {
    const char* text;
    int count;
    int start;
} argumetsForFunctionInThread;

int error_check(int code, char* inscription) {
    if (code != SUCCESS) {
        perror(inscription);
        return code;
    }
    return SUCCESS;
}

int destroySems(int number) {
    for (int i = 0; i < number; i++) {
        errno = sem_destroy(&sems[i]);
        if (error_check(errno, "Destroying semaphore error") != SUCCESS) {
            return errno;
        }
    }
    return SUCCESS;
}

int initializeSems() {
    for (int i = 0; i < NUMBER_OF_SEMAPHORES; ++i) {
        errno = sem_init(&sems[i], 0, i);
        if (error_check(errno, "Sem_init error") != SUCCESS) {
            destroy_sems(i);
            return errno;
        }
    }
    return SUCCESS;
}
int semaphoreWait(int num) {
    errno = sem_wait(&sems[num]);
    if (error_check(errno, "Semaphore wait error") != SUCCESS) {
        return errno;
    }
    return SUCCESS;
}

int semaphorePost(int num) {
    errno = sem_post(&sems[num]);
    if (error_check(errno, "Semaphore post error") != SUCCESS) {
        return errno;
    }
    return SUCCESS;
}

void* printTextInThread(void* param) {
    Data data = *(Data*)param;
    errno = SUCCESS;
    int this_sem = 0,
        next_sem = 0;

    for (int i = 0; i < NUMBER_OF_LINES; i++) {
        this_sem = (data.num_queue + 1) % NUMBER_OF_SEMAPHORES;
        next_sem = (this_sem + 1) % NUMBER_OF_SEMAPHORES;
        errno = semaphore_wait(this_sem);
        if (error_check(errno, "Semaphore wait error") != SUCCESS) {
            return NULL;
        }

        printf("%s %d\n", data.str, i);

        errno = semaphore_post(next_sem);
        if (error_check(errno, "Semaphore post error") != SUCCESS) {
            return NULL;
        }
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    pthread_t thread;
    argumetsForFunctionInThread newThread = { "Hello, I'm new thread\n", 10, 1};
    argumetsForFunctionInThread mainThread = { "Hello, I'm main thread\n", 10, 0};
    errno = initialize_sems();
    if (errno != SUCCESS) {
        exit(errno);
    }

    errno = pthread_create(&thread, NULL, printTextInThread, &newThread);
    if (error_check(errno, "Creating thread error") != SUCCESS) {
        destroy_sems(NUMBER_OF_SEMAPHORES);
        exit(errno);
    }

    printTextInThread(&mainThread);

    errno = pthread_join(thread, NULL);
    if (error_check(errno, "Joining thread error") != SUCCESS) {
        destroy_sems(NUMBER_OF_SEMAPHORES);
        exit(errno);
    }

    destroy_sems(NUMBER_OF_SEMAPHORES);
    return SUCCESS;
}
