#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define THREAD_POOL_SIZE 4

typedef struct thread_parameter {
    int size;
    char** strings;
} thread_parameter;

void* print(void* arg) {
    thread_parameter* threads_parameter = (thread_parameter*)arg;
    int index_for_thread = pthread_self() - 2;
    for (int i = 0; i < (threads_parameter + index_for_thread)->size; ++i) {
        printf("%s\n", *((threads_parameter + index_for_thread)->strings + i));
    }

    pthread_exit(arg);
}
int main(int argc, char* argv[]) {
    pthread_t threads[THREAD_POOL_SIZE];

    char* strings[] = { "1", "2", "3", "4", "5", "6", "7", "8" };

    thread_parameter threads_parameter[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
        threads_parameter[i].size = 2;
        threads_parameter[i].strings = strings + 2 * i;
    }

    for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
        if (pthread_create(&threads[i], NULL, print, (void*)threads_parameter) >
            perror("pthread_create");
            exit(EXIT_FAILURE);
    }
}

pthread_exit(NULL);
}
