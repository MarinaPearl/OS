#ifndef LAB7_CP_R_H
#define LAB7_CP_R_H

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
#include <stddef.h>
#include <fcntl.h>

#define NUMBER_INPUT_ARGUMENTS 3
#define DESCRIPTION_INPUT_ARGUMENTS "The number of arguments is not equal to 2. The first  argument is the sources path.The second argument is the destination path.\n"
#define SIZE_END_LINE 1
#define SUCCESS 0
#define FAILURE (-1)
#define TIMEOUT_LIMIT_OPEN_FILES 1
#define STRING_EQUALITY 0
#define SUCCESS_FILE_DESCRIPTOR 0
#define COPE_BUF_SIZE 4096
#define NOT_FILE (-2)
#define SRC_PATH 1
#define DEST_PATH 2
#define MIN_SIZE_FILE 0


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

copyInfo *createCopyInfo(char *srcPath, char *destPath, mode_t mode);
void destroyResources();
void freeResources(copyInfo *info);
int initializeStartResources(char** srcBuf, char** destBuf, size_t srcPathLen, size_t destPathLen);
int makeDir(copyInfo *info);
DIR *openDir(const char *dirName);
int readDir(DIR *dir, struct dirent *entry, struct dirent **result);
char *appendPath(char *dir, char *newName, size_t maxLength);
int checkFile(copyInfo *info);
int findType(mode_t mode);
void closeDir(DIR *dir);
int createNewPath(char* srcNext, char* destNext, copyInfo* infoNext, copyInfo* info, size_t maxPathLength, struct dirent* entry);
int copyDir(copyInfo *info);
void *copyDirInThread(void *arg);
int openFile(char *file);
int createFile(char *file, mode_t mode);
int copyFile(copyInfo *info);
void *copyFileInThread(void *arg);
int createThreadForDir(copyInfo *info);
int createThreadForFile(copyInfo *info);
int startCp_R(const char* src, const char* dest);

#endif //LAB7_CP_R_H
