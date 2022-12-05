#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define NUMBER_INPUT_ARGUMENTS 3
#define DESCRIPTION_INPUT_ARGUMENTS "The number of arguments is not equal to 2. The first  argument is the sources path.The second argument is the destination path.\n"
#define SIZE_END_LINE 1
#define SUCCESS 0
#define FAILURE (-1)
#define TIMEOUT_LIMIT_OPEN_DIR 1

pthread_attr_t attr;

typedef struct {
    char *srcPath;
    char *destPath;
    mode_t mode;
} copyInfo;

copyInfo *createCopyInfo(char *srcPath, char *destPath, mode_t mode) {
    copyInfo *copy = (copyInfo *) malloc(sizeof(copyInfo));
    copy->srcPath = srcPath;
    copy->destPath = destPath;
    copy->mode = mode;
    return copy;
}

void destroyResources() {
    errno = pthread_attr_destroy(&attr);
    if (errno != SUCCESS) {
        perror("Error in destroy attr");
    }
}

void freeResourses(copyInfo *info) {
    if (info != NULL) {
        free(info->srcPath);
        free(info->destPath);
        free(info);
    }
}

int initializeResources() {
    errno = pthread_attr_init(&attr);
    if (errno != SUCCESS) {
        perror("Error in attr init");
        return FAILURE;
    }
    errno = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (errno != SUCCESS) {
        perror("Error in set detach state");
        destroyResources();
        return FAILURE;
    }
    return SUCCESS;
}

int makeDir(copyInfo *info) {
    mkdir(info->destPath, info->mode);
    if (errno != SUCCESS && errno != EEXIST) {
        perror("Error in mkdir");
        return FAILURE;
    }
    return SUCCESS;
}

DIR *openDir(const char *dirName) {
    bool limitDirOpen = false;
    while (true) {
        if (limitDirOpen == true) {
            sleep(TIMEOUT_LIMIT_OPEN_DIR);
        }
        DIR *dir = opendir(dirName);
        if (dir != NULL) {
            return dir;
        }
        if (errno != EMFILE)
            return dir;
        limitDirOpen = true;
    }
}

int copyDir(copyInfo *info) {
    size_t maxPathLength = (size_t) pathconf(info->srcPath, _PC_PATH_MAX);
    int ret = makeDir(info);
    if (ret != SUCCESS) {
        return FAILURE;
    }
    DIR* dir = openDir(info->srcPath);
    if (dir == NULL) {
        return FAILURE;
    }

    printf("%ld\n", maxPathLength);
}

void *copyDirInThread(void *arg) {
    copyInfo *info = (copyInfo *) arg;
    int err = copyDir(info);
    if (err != SUCCESS) {
        fprintf(stderr, "Error in this files : %s %s\n", info->srcPath, info->destPath);
    }
    freeResourses(info);
    pthread_exit(NULL);
}


int createThreadForDir(copyInfo *info) {
    pthread_t ntid;
    errno = pthread_create(&ntid, NULL, copyDirInThread, (void *) info);
    if (errno != SUCCESS) {
        perror("Error in pthread create");
        return FAILURE;
    }
    return SUCCESS;
}

int main(int argc, const char **argv) {
    if (argc != NUMBER_INPUT_ARGUMENTS) {
        printf(DESCRIPTION_INPUT_ARGUMENTS);
        exit(EXIT_FAILURE);
    }
    int retInitRes = initializeResources();
    if (retInitRes != SUCCESS) {
        exit(EXIT_FAILURE);
    }
    if (atexit(destroyResources) != SUCCESS) {
        perror("Error in atexit");
        exit(EXIT_FAILURE);
    }
    size_t srcPathLen = strlen(argv[1]);
    size_t destPathLen = strlen(argv[2]);
    char *srcBuf = (char *) malloc(srcPathLen * sizeof(char) + SIZE_END_LINE);
    if (srcBuf == NULL) {
        perror("Error in malloc");
        exit(EXIT_FAILURE);
    }
    char *destBuf = (char *) malloc(destPathLen * sizeof(char) + SIZE_END_LINE);
    if (destBuf == NULL) {
        perror("Error in malloc");
        exit(EXIT_FAILURE);
    }
    strcpy(srcBuf, argv[1]);
    strcpy(destBuf, argv[2]);
    struct dirent dirent;
    struct stat structStat;
    if (stat(srcBuf, &structStat) != SUCCESS) {
        perror("Error in stat");
        exit(EXIT_FAILURE);
    }
    copyInfo *copy = createCopyInfo(srcBuf, destBuf, structStat.st_mode);
    createThreadForDir(copy);

    //free(srcBuf);
    //free(destBuf);
    pthread_exit(NULL);
}