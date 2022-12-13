#ifndef LAB7_CP_R_H
#define LAB7_CP_R_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUMBER_INPUT_ARGUMENTS 3
#define DESCRIPTION_INPUT_ARGUMENTS "The number of arguments is not equal to 2. The first  argument is the sources path.The second argument is the destination path.\n"
#define SRC_PATH 1
#define DEST_PATH 2
#define SUCCESS 0

int startCp_R(const char* src, const char* dest);

#endif //LAB7_CP_R_H
