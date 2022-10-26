#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define THREAD_POOL_SIZE 4
#define SUCCESS 0

void* printStrings(void* arguments) {
    char** value = (char**)arguments;
    for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
        printf("%s\n", value[i]);
    }
    pthread_exit(NULL);
}

void printErrorandAbortProgram(int valueError, char* msg) {
    char buf[1024];
    strerror_r(valueError, buf, sizeof(buf));
    fprintf(stderr, "%s cause : %s\n", msg, buf);
    exit(EXIT_FAILURE);
}

void releaseResourses(int index, pthread_t* ntid) {
    for (int i = 0; i < index; ++i) {
        int code = pthread_join(ntid[i], NULL);
        if (code != SUCCESS) {
            printErrorandAbortProgram(code, "Error in the join function");
        }
    }
}

int main() {
    char* stringsfotFunctiomInThread[THREAD_POOL_SIZE][THREAD_POOL_SIZE] = {
        {"it is thread 1, string  1", "it is thread 1, string  2", "it is thread 1, string  3", "it is thread 1, string  4"},
        {"it is thread 2, string  1", "it is thread 2, string  2", "it is thread 2, string  3", "it is thread 2, string  4"},
        {"it is thread 3, string  1", "it is thread 3, string  2", "it is thread 3, string  3", "it is thread 3, string  4"},
        {"it is thread 4, string  1", "it is thread 4, string  2", "it is thread 4, string  3", "it is thread 4, string  4"}
    };

    pthread_t ntid[THREAD_POOL_SIZE];
    int code;
    for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
        code = pthread_create(&ntid[i], NULL, printStrings,(void*)&stringsfotFunctiomInThread[i]);
        if (code != SUCCESS) {
            releaseResourses(i, ntid);
            printErrorandAbortProgram(code, "Error in create function");
        }
    }

    for (int i = 0; i < THREAD_POOL_SIZE; ++i) {
        int code = pthread_join(ntid[i], NULL);
        if (code != SUCCESS) {
            printErrorandAbortProgram(code, "Error in the join function");
        }
    }

    return SUCCESS;
}