#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 
#include <string.h>
#define SUCCESS 0
typedef struct argumentsForFunction {
    const char* text;
    int count;
} argumetsForFunctionInThread;

void* printTextInThread(void* args) {
    argumetsForFunctionInThread* value = (argumetsForFunctionInThread*)args;
    for (int i = 0; i < value->count; ++i) {
        printf("%s\n", value->text);
    }
    return NULL;
}
void posixError(int valueError, const char* msg) {
    fprintf(stderr, "%s cause : %s\n", msg, strerror(valueError));
    exit(EXIT_FAILURE);
}
int main() {
    pthread_t ntid;
    argumetsForFunctionInThread newThread = { "Hello, I'm new thread\n", 10 };
    argumetsForFunctionInThread mainThread = { "Hello, I'm main thread\n", 10 };
    int err = pthread_create(&ntid, NULL, printTextInThread, (void*)&newThread);
    if (err != SUCCESS) {
        posixError(err, "unable to create thread");
    }
    err = pthread_join(ntid, NULL);
    if (err != SUCCESS) {
        posixError(err, "it is impossible to continue the main stream");
    }
    printTextInThread((void*)&mainThread);
    return EXIT_SUCCESS;
}
