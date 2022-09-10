#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#define SUCCESS 0
typedef struct argumentsForPrintText {
	const char* text;
	int count;
} argumentsForPrintTextInThread;
void* printText(void* args) {
	argumentsForPrintTextInThread* value = (argumentsForPrintTextInThread*)args;
	for (int i = 0; i < value->count; ++i) {
		printf("%s\n", value->text);
	}
	return NULL;
}
void printError(int valueError, const char* msg) {
	fprintf(stderr, "%s cause : %s\n", msg, strerror(valueError));
	exit(EXIT_FAILURE);
}
int main() {
	pthread_t ntid;

	argumentsForPrintTextInThread newThread = { "Hello, I'm new thread\n", 10 };
	argumentsForPrintTextInThread mainThread = { "Hello, I'm main thread\n", 10 };

	int err = pthread_create(&ntid, NULL, printText, (void*)&newThread);

	if (err != SUCCESS) {
		printError(err, "unable to create thread");
	}
	printText((void*)&mainThread);
	pthread_exit(NULL);
}
