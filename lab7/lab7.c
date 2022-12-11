#include "cp_r.h"

int main(int argc, const char **argv) {
    if (argc != NUMBER_INPUT_ARGUMENTS) {
        printf(DESCRIPTION_INPUT_ARGUMENTS);
        exit(EXIT_FAILURE);
    }
    int retCp_r = startCp_R(argv[SRC_PATH], argv[DEST_PATH]);
    if (retCp_r != SUCCESS) {
        exit(EXIT_FAILURE);
    }
    pthread_exit(NULL);
}