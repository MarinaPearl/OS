#include "cp_r.h"
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <stddef.h>
#include <fcntl.h>

#define SIZE_END_LINE 1
#define FAILURE (-1)
#define TIMEOUT_LIMIT_OPEN_FILES 1
#define STRING_EQUALITY 0
#define SUCCESS_FILE_DESCRIPTOR 0
#define COPE_BUF_SIZE 4096
#define NOT_FILE (-2)
#define MIN_SIZE_FILE 0
#define SUCCESS 0
#define FAILURE_AFTER_MALLOC_INPUT (-2)

pthread_attr_t attr;
char *destinationPath;

typedef struct {
    char *srcPath;
    char *destPath;
    mode_t mode;
} copyInfo;

enum typeFile {
    type_DIRECTORY,
    type_REGULAR_FILE,
    type_OTHER
};

copyInfo *createCopyInfo(char *srcPath, char *destPath, mode_t mode) {
    copyInfo *copy = (copyInfo *) malloc(sizeof(copyInfo));
    if (copy == NULL) {
        perror("Error in malloc");
        return copy;
    }
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

void freeResources(copyInfo *info) {
    if (info != NULL) {
        free(info->srcPath);
        free(info->destPath);
        free(info);
    }
}

void destroyResourcesInExit() {
    free(destinationPath);
    if (errno != SUCCESS) {
        perror("Error in destroy attr");
    }
}

int initializeStartResources(char **srcBuf, char **destBuf, size_t srcPathLen, size_t destPathLen) {
    *srcBuf = (char *) malloc(srcPathLen * sizeof(char) + SIZE_END_LINE);
    if (srcBuf == NULL) {
        perror("Error in malloc");
        return FAILURE;
    }
    *destBuf = (char *) malloc(destPathLen * sizeof(char) + SIZE_END_LINE);
    if (destBuf == NULL) {
        free(srcBuf);
        perror("Error in malloc");
        return FAILURE;
    }
    errno = pthread_attr_init(&attr);
    if (errno != SUCCESS) {
        perror("Error in attr init");
        return FAILURE_AFTER_MALLOC_INPUT;
    }
    errno = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (errno != SUCCESS) {
        perror("Error in set detach state");
        destroyResources();
        return FAILURE_AFTER_MALLOC_INPUT;
    }
    destinationPath = (char *) malloc(sizeof(char) * destPathLen + SIZE_END_LINE);
    if (destinationPath == NULL) {
        perror("Error in malloc");
        destroyResources();
        return FAILURE_AFTER_MALLOC_INPUT;
    }
    return SUCCESS;
}

int makeDir(copyInfo *info) {
    mkdir(info->destPath, info->mode);
    if (errno != SUCCESS && errno != EEXIST) {
        perror("Error in mkdir");
        return FAILURE;
    }
    errno = SUCCESS;
    return SUCCESS;
}

DIR *openDir(const char *dirName) {
    bool limitDirOpen = false;
    while (true) {
        if (limitDirOpen == true) {
            sleep(TIMEOUT_LIMIT_OPEN_FILES);
        }
        DIR *dir = opendir(dirName);
        if (dir != NULL) {
            return dir;
        }
        if (errno != EMFILE) {
            return dir;
        }
        limitDirOpen = true;
    }
}

int readDir(DIR *dir, struct dirent *entry, struct dirent **result) {
    errno = readdir_r(dir, entry, result);
    return errno;
}

bool equateString(char *path, char *unsuitablePath) {
    return strcmp(path, unsuitablePath) == STRING_EQUALITY;
}

char *appendPath(char *dir, char *newName, size_t maxLength) {
    char *path = (char *) malloc(maxLength * sizeof(char));
    if (path == NULL) {
        perror("Error in malloc");
        return path;
    }
    strcpy(path, dir);
    size_t pathLen = strlen(path);
    if (pathLen >= maxLength) {
        fprintf(stderr, "The maximum path length has been reached\n");
        return NULL;
    }
    path[pathLen] = '/';
    ++pathLen;
    path[pathLen] = '\0';
    path = strncat(path, newName, maxLength - pathLen);
    return path;
}

int findType(mode_t mode) {
    if (S_ISDIR(mode)) {
        return type_DIRECTORY;
    }
    if (S_ISREG(mode)) {
        return type_REGULAR_FILE;
    }
    return type_OTHER;
}

int startCopy(copyInfo *info) {
    int type = findType(info->mode);
    int retCreate;
    switch (type) {
        case type_DIRECTORY:
            retCreate = createThreadForDir(info);
            if (retCreate != SUCCESS) {
                return FAILURE;
            }
            break;
        case type_REGULAR_FILE:
            retCreate = createThreadForFile(info);
            if (retCreate != SUCCESS) {
                return FAILURE;
            }
            break;
        case type_OTHER:
            return NOT_FILE;
    }
    return SUCCESS;
}

void closeDir(DIR *dir) {
    closedir(dir);
    if (errno != SUCCESS) {
        perror("Error in close dir");
    }
}

void freeNewPath(char* srcNext, char* destNext) {
    free(srcNext);
    free(destNext);
}

int createNewPath(char *srcNext, char *destNext, copyInfo *infoNext, copyInfo *info, size_t maxPathLength,
                  struct dirent *entry) {
    struct stat structStat;
    srcNext = appendPath(info->srcPath, entry->d_name, maxPathLength);
    if (srcNext == NULL) {
        return FAILURE;
    }
    destNext = appendPath(info->destPath, entry->d_name, maxPathLength);
    if (destNext == NULL) {
        free(srcNext);
        return FAILURE;
    }
    if (lstat(srcNext, &structStat) != SUCCESS) {
        perror("Error in stat");
        freeNewPath(srcNext, destNext);
        errno = SUCCESS;
        return FAILURE;
    }
    infoNext = createCopyInfo(srcNext, destNext, structStat.st_mode);
    if (infoNext == NULL) {
        freeNewPath(srcNext, destNext);
        return FAILURE;
    }
    int retCheck = startCopy(infoNext);
    if (retCheck != SUCCESS) {
        freeResources(infoNext);
        return retCheck;
    }
}

int copyDir(copyInfo *info) {
    int ret;
    size_t maxPathLength = (size_t) pathconf(info->srcPath, _PC_PATH_MAX);
    ret = makeDir(info);
    if (ret != SUCCESS) {
        return FAILURE;
    }
    DIR *dir = openDir(info->srcPath);
    if (dir == NULL) {
        return FAILURE;
    }
    size_t entryLen = offsetof(struct dirent, d_name) +pathconf(info->srcPath, _PC_NAME_MAX) + SIZE_END_LINE;
    struct dirent *entry = (struct dirent *) malloc(entryLen);
    struct dirent *result;
    if (entry == NULL) {
        perror("Error in malloc\n");
        closeDir(dir);
        return FAILURE;
    }
    while ((ret = readDir(dir, entry, &result)) == SUCCESS) {
        if (result == NULL) {
            break;
        }
        if (equateString(entry->d_name, ".") || equateString(entry->d_name, "..") ||
            equateString(info->srcPath, destinationPath)) {
            continue;
        }
        char *srcNext, *destNext;
        copyInfo *infoNext;
        ret = createNewPath(srcNext, destNext, infoNext, info, maxPathLength, entry);
        if (ret == FAILURE) {
            break;
        }
    }
    free(entry);
    closeDir(dir);
    return ret;
}

void *copyDirInThread(void *arg) {
    copyInfo *info = (copyInfo *) arg;
    int err = copyDir(info);
    if (err == FAILURE) {
        fprintf(stderr, "Error in this files : %s %s\n", info->srcPath, info->destPath);
    }
    freeResources(info);
    pthread_exit(NULL);
}

int openFile(char *file) {
    bool fdLimit = false;
    while (true) {
        if (fdLimit) {
            sleep(TIMEOUT_LIMIT_OPEN_FILES);
        }
        int fd = open(file, O_RDONLY);
        if (fd >= SUCCESS_FILE_DESCRIPTOR) {
            return fd;
        }
        if (errno != EMFILE && errno != SUCCESS) {
            perror("Error in open");
            return FAILURE;
        }
        fdLimit = true;
    }
}

int createFile(char *file, mode_t mode) {
    bool fdLimit = false;
    while (true) {
        if (fdLimit) {
            sleep(TIMEOUT_LIMIT_OPEN_FILES);
        }
        int fd = creat(file, mode);
        if (fd >= SUCCESS_FILE_DESCRIPTOR) {
            return fd;
        }
        if (errno != EMFILE && errno != SUCCESS) {
            perror("Error in open");
            return FAILURE;
        }
        fdLimit = true;
    }
}

void closeFd(int srcFd, int destFd) {
    close(srcFd);
    close(destFd);
}

int copyFile(copyInfo *info) {
    int srcFd = openFile(info->srcPath);
    if (srcFd == FAILURE) {
        return FAILURE;
    }
    int destFd = createFile(info->destPath, info->mode);
    if (destFd == FAILURE) {
        close(srcFd);
        return FAILURE;
    }
    char *buffer[COPE_BUF_SIZE];
    ssize_t readBytes;
    while ((readBytes = read(srcFd, (void *) buffer, COPE_BUF_SIZE)) > MIN_SIZE_FILE) {
        ssize_t writtenBytes = write(destFd, (void *) buffer, readBytes);
        if (errno != SUCCESS) {
            perror("Error in write");
            closeFd(srcFd, destFd);
            return FAILURE;
        }
    }
    closeFd(srcFd, destFd);
    if (errno != SUCCESS) {
        perror("Error in read");
        return FAILURE;
    }
    return SUCCESS;
}

void *copyFileInThread(void *arg) {
    copyInfo *info = (copyInfo *) arg;
    int err = copyFile(info);
    if (err != SUCCESS) {
        fprintf(stderr, "Error in this files : %s %s\n", info->srcPath, info->destPath);
    }
    freeResources(info);
    pthread_exit(NULL);
}

int createThreadForDir(copyInfo *info) {
    pthread_t ntid;
    errno = pthread_create(&ntid, &attr, copyDirInThread, (void *) info);
    if (errno != SUCCESS) {
        perror("Error in pthread create");
        return FAILURE;
    }
    return SUCCESS;
}

int createThreadForFile(copyInfo *info) {
    pthread_t ntid;
    errno = pthread_create(&ntid, &attr, copyFileInThread, (void *) info);
    if (errno != SUCCESS) {
        perror("Error in pthread create");
        return FAILURE;
    }
    return SUCCESS;
}

void freeInputArgs(char *src, char *dest) {
    free(src);
    free(dest);
}

int startCp_R(const char *src, const char *dest) {
    size_t srcPathLen = strlen(src);
    size_t destPathLen = strlen(dest);
    char *srcBuf;
    char *destBuf;
    int retInitRes = initializeStartResources(&srcBuf, &destBuf, srcPathLen, destPathLen);
    if (retInitRes != SUCCESS) {
        if (retInitRes == FAILURE_AFTER_MALLOC_INPUT) {
            freeInputArgs(srcBuf, destBuf);
        }
        return FAILURE;
    }
    if (atexit(destroyResourcesInExit) != SUCCESS) {
        freeInputArgs(srcBuf, destBuf);
        perror("Error in atexit");
        return FAILURE;
    }
    strcpy(srcBuf, src);
    strcpy(destBuf, dest);
    strcpy(destinationPath, dest);
    struct stat structStat;
    if (lstat(srcBuf, &structStat) != SUCCESS) {
        freeInputArgs(srcBuf, destBuf);
        perror("Error in stat");
        return FAILURE;
    }
    copyInfo *copy = createCopyInfo(srcBuf, destBuf, structStat.st_mode);
    if (copy == NULL) {
        freeInputArgs(srcBuf, destBuf);
        return FAILURE;
    }
    int retCreate = createThreadForDir(copy);
    if (retCreate != SUCCESS) {
        freeResources(copy);
        return FAILURE;
    }
}
