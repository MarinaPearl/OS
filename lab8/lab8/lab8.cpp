#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#define RIGHT_NUMBER_ARGUMENTS 3
#define STRING_TO_INT_FAILURE 0
#define COMPARISON_SUCCESS 0
#define MAX_THREAD 1024
#define MAX_ITERATIONS INT_MAX
#define CREATE_SUCCESS 0 
#define JOIN_SUCCESS 0

typedef struct inputArguments {
	int countThread;
	int countIterations;
} inputArguments;

typedef struct argumentsForFunctionInThread {
	int startIteration;
	int endIteration;
	double partialSum;
} argumentsForFunctionInThread;

enum parsInputArguments {
	parsInputArguments_SUCCESS,
	parsInputArguments_WRONG_COUNT_OF_ARGUMENTS,
	parsInputArguments_VALUE_THREAD_NOT_NUMBER,
	parsInputArguments_WRONG_COUNT_THREAD,
	parsInputArguments_VALUE_INTERATIONS_NOT_NUMBER,
	parsInputArguments_WRONG_COUNT_ITERATIONS
};

int parsingInputArguments(int argc, char** argv, inputArguments* arguments) {
	if (argc != RIGHT_NUMBER_ARGUMENTS) {
		return parsInputArguments_WRONG_COUNT_OF_ARGUMENTS;
	}

	int countThread = atoi(argv[1]);
	if (countThread == STRING_TO_INT_FAILURE && strcmp(argv[1], "0") != COMPARISON_SUCCESS) {
		return parsInputArguments_VALUE_THREAD_NOT_NUMBER;
	}
	if (countThread < 0 || countThread > MAX_THREAD) {
		return parsInputArguments_WRONG_COUNT_THREAD;
	}
	arguments->countThread = countThread;

	int countIterations = atoi(argv[2]);
	if (countIterations == STRING_TO_INT_FAILURE && strcmp(argv[2], "0") != COMPARISON_SUCCESS) {
		return parsInputArguments_VALUE_INTERATIONS_NOT_NUMBER;
	}
	if (countIterations < 0 || countIterations > MAX_ITERATIONS) {
		return parsInputArguments_WRONG_COUNT_ITERATIONS;
	}
	arguments->countIterations = countIterations;

	return parsInputArguments_SUCCESS;
}

void inputError(int code) {
	switch (code)
	{
	case parsInputArguments_WRONG_COUNT_OF_ARGUMENTS:
		fprintf(stderr, "introduced a wrong number of arguments\n");
		break;
	case parsInputArguments_VALUE_THREAD_NOT_NUMBER:
		fprintf(stderr, "thread argument is not an number\n");
		break;
	case parsInputArguments_WRONG_COUNT_THREAD:
		fprintf(stderr, "number of threads entered incorrectly\n");
		break;
	case parsInputArguments_VALUE_INTERATIONS_NOT_NUMBER:
		fprintf(stderr, "iteration argument is not an number\n");
		break;
	case parsInputArguments_WRONG_COUNT_ITERATIONS:
		fprintf(stderr, "number of iterations entered incorrectly\n");
		break;
	}
	exit(EXIT_FAILURE);
}

void divisionIteration(int* array, inputArguments args) {
	int countInterartionsInOneThread = args.countIterations / args.countThread;
	for (int i = 0; i < args.countThread; ++i) {
		array[i] = countInterartionsInOneThread;
	}

	int restIteration = args.countIterations % args.countThread;
	if (restIteration != 0)
		for (int i = 0; i < restIteration; ++i) {
			++array[i];
		}
}

void fillingGeneralArrayForFunctionInThread(argumentsForFunctionInThread* array, inputArguments args, int* count) {
	for (int i = 0; i < args.countThread; ++i) {
		if (i == 0) {
			array[i].startIteration = 0;
			array[i].endIteration = count[i];
		}
		else {
			array[i].startIteration = array[i - 1].endIteration;
			array[i].endIteration = array[i - 1].endIteration + count[i];
		}
		array[i].partialSum = 0;
	}
}

void  processingThreadError(int valueError, const char* msg) {
	fprintf(stderr, "%s cause : %s\n", msg, strerror(valueError));
	exit(EXIT_FAILURE);
}

void* calculatePartialSum(void* args) {
	argumentsForFunctionInThread* value = (argumentsForFunctionInThread*)args;
	for (int i = value->startIteration; i < value->endIteration; ++i) {
		if (i % 2 == 0) {
			value->partialSum += 1.0 / (i * 2.0 + 1.0);

		}
		else {
			value->partialSum -= 1.0 / (i * 2.0 + 1.0);
		}
	}
	pthread_exit(&value->partialSum);
}

void createThread(pthread_t* ntid, argumentsForFunctionInThread* array, inputArguments args) {
	for (int i = 0; i < args.countThread; ++i) {
		int err = pthread_create(&ntid[i], NULL, calculatePartialSum, (void*)&array[i]);
		if (err != CREATE_SUCCESS) {
			processingThreadError(err, "unable to create thread");
		}
	}
}

void collectionPartialSumm(pthread_t* ntid, argumentsForFunctionInThread* array, inputArguments args, double* sum) {
	*sum = 0;
	void* resultInThread;
	for (int i = 0; i < args.countThread; ++i) {
		int err = pthread_join(ntid[i], &resultInThread);
		if (err != JOIN_SUCCESS) {
			processingThreadError(err, "error while waiting for thread");
		}
		*sum += *(double*)resultInThread;
	}
	*sum *= 4;
}

void printPi(double* pi) {
	fprintf(stdout, "pi = %.20f\n", *pi);
}

void calculationPI(inputArguments args) {
	int countIterationInOneThread[args.countThread];
	divisionIteration(countIterationInOneThread, args);

	argumentsForFunctionInThread arrayArguments[args.countThread];
	fillingGeneralArrayForFunctionInThread(arrayArguments, args, countIterationInOneThread);

	pthread_t ntid[args.countThread];
	createThread(ntid, arrayArguments, args);

	double pi = 0;
	collectionPartialSumm(ntid, arrayArguments, args, &pi);
	printPi(&pi);
}

int main(int argc, char** argv) {
	inputArguments args;

	int code = parsingInputArguments(argc, argv, &args);
	if (code != parsInputArguments_SUCCESS) {
		inputError(code);
	}

	calculationPI(args);

	exit(EXIT_SUCCESS);
}