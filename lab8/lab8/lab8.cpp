#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#define RIGHT_NUMBER_ARGUMENTS 3
#define STRING_TO_INT_FAILURE 0
#define COMPARISON_SUCCESS 0
#define MAX_THREAD 512
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

enum resultOfCheckInputArguments {
    inputArguments_SUCCESS,
    inputArguments_WRONG_COUNT_OF_ARGUMENTS,
    inputArguments_VALUE_THREAD_NOT_NUMBER,
    inputArguments_WRONG_COUNT_THREAD,
    inputArguments_VALUE_INTERATIONS_NOT_NUMBER,
    inputArguments_WRONG_COUNT_ITERATIONS
};

int checkInputArguments(int argc, char** argv, inputArguments* arguments) {
    if (argc != RIGHT_NUMBER_ARGUMENTS) {
        return inputArguments_WRONG_COUNT_OF_ARGUMENTS;
    }

    int countThread = atoi(argv[1]);
    if (countThread == STRING_TO_INT_FAILURE && strcmp(argv[1], "0") != COMPARISON_SUCCESS) {
        return inputArguments_VALUE_THREAD_NOT_NUMBER;
    }
    if (countThread <= 0 || countThread > MAX_THREAD) {
        return inputArguments_WRONG_COUNT_THREAD;
    }
    arguments->countThread = countThread;

    int countIterations = atoi(argv[2]);
    if (countIterations == STRING_TO_INT_FAILURE && strcmp(argv[2], "0") != COMPARISON_SUCCESS) {
        return inputArguments_VALUE_INTERATIONS_NOT_NUMBER;
    }
    if (countIterations <= 0 || countIterations > MAX_ITERATIONS) {
        return inputArguments_WRONG_COUNT_ITERATIONS;
    }
    arguments->countIterations = countIterations;

    return inputArguments_SUCCESS;
}

void printErrorInputArgsAndTerminateProgram(int code) {
    switch (code) {
        case inputArguments_WRONG_COUNT_OF_ARGUMENTS:
             fprintf(stderr, "Error : introduced a wrong number of arguments\n");
             break;
        case inputArguments_VALUE_THREAD_NOT_NUMBER:
             fprintf(stderr, "Error : thread argument is not a number\n");
             break;
        case inputArguments_WRONG_COUNT_THREAD:
             fprintf(stderr, "Error : number of threads entered incorrectly\n");
             break;
        case inputArguments_VALUE_INTERATIONS_NOT_NUMBER:
             fprintf(stderr, "Error : iteration argument is not a number\n");
             break;
        case inputArguments_WRONG_COUNT_ITERATIONS:
             fprintf(stderr, "Error : number of iterations entered incorrectly\n");
             break;
        default:
             fprintf(stderr, "Error : error not found\n");
             break;
    }
    exit(EXIT_FAILURE);
}

void distributeIterationsInToThreads(int* array, inputArguments args) {
    int countInterartionsInOneThread = args.countIterations / args.countThread;
    for (int i = 0; i < args.countThread; ++i) {
        array[i] = countInterartionsInOneThread;
    }

    int restIteration = args.countIterations % args.countThread;
    if (restIteration != 0) {
        for (int i = 0; i < restIteration; ++i) {
            ++array[i];
        }
    }
}

void fillGeneralArrayForFunctionInThread(argumentsForFunctionInThread* array, inputArguments args, int* offset) {
    int sumOffset = 0;
    for (int i = 0; i < args.countThread; ++i) {
        array[i].startIteration = sumOffset;
        sumOffset += offset[i];
        array[i].endIteration = sumOffset;
        array[i].partialSum = 0.0;
    }
}

void threadError(int valueError, const char* msg) {
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
            threadError(err, "unable to create thread");
        }
    }
}

void addPartialSums(pthread_t* ntid, argumentsForFunctionInThread* array, inputArguments args, double* sum) {
    *sum = 0;
    void* resultInThread;
    for (int i = 0; i < args.countThread; ++i) {
        int err = pthread_join(ntid[i], &resultInThread);
        if (err != JOIN_SUCCESS) {
            threadError(err, "error while waiting for thread");
        }
        *sum += *(double*)resultInThread;
    }
    *sum *= 4;
}

void printPi(double* pi) {
    fprintf(stdout, "pi = %.20f\n", *pi);
}

void calculatePI(inputArguments args) {
    int countIterationInOneThread[args.countThread];
    distributeIterationsInToThreads(countIterationInOneThread, args);

    argumentsForFunctionInThread arrayArguments[args.countThread];
    fillGeneralArrayForFunctionInThread(arrayArguments, args, countIterationInOneThread);

    pthread_t ntid[args.countThread];
    createThread(ntid, arrayArguments, args);

    double pi = 0;
    calculatePartialSum(ntid, arrayArguments, args, &pi);
    printPi(&pi);
}

int main(int argc, char** argv) {
    inputArguments args;
    int code = checkInputArguments(argc, argv, &args);
    if (code != inputArguments_SUCCESS) {
        printErrorInputArgsAndTerminateProgram(code);
    }
    calculatePI(args);
    return EXIT_SUCCESS;
}