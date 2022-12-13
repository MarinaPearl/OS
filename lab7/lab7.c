#include "cp_r.h"

#define NUMBER_INPUT_ARGUMENTS 3
#define DESCRIPTION_INPUT_ARGUMENTS "The number of arguments is not equal to 2. The first  argument is the sources path.The second argument is the destination path.\n"
#define SRC_PATH 1
#define DEST_PATH 2
#define SUCCESS_CP_R 0

int main(int argc, const char **argv) {
    if (argc != NUMBER_INPUT_ARGUMENTS) {
        printf(DESCRIPTION_INPUT_ARGUMENTS);
        exit(EXIT_FAILURE);
    }
    int retCp_r = startCp_R(argv[SRC_PATH], argv[DEST_PATH]);
    if (retCp_r != SUCCESS_CP_R) {
        exit(EXIT_FAILURE);
    }
    pthread_exit(NULL);
}