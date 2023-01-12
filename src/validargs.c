#include <stdlib.h>

#include "argo.h"
#include "global.h"
#include "debug.h"

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the various options that were specified will be
 * encoded in the global variable 'global_options', where it will be
 * accessible elsewhere in the program.  For details of the required
 * encoding, see the assignment handout.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * @modifies global variable "global_options" to contain an encoded representation
 * of the selected program options.
 */
int cmp(char *a, char *b) {
    if (a == NULL || b == NULL)
        return 11;
    char ap = *a;
    char bp = *b;
    int i = 1;
    while (ap != '\0') {
        if (bp == '\0')
            return 1;
        else if (bp > ap)
            return  -1;
        else if (ap > bp)
            return 1;
        ap = *(a + i);
        bp = *(b + i);
        i = i + 1;
    }
    if (bp == '\0')
        return 0;
    return -1;
}
int validDigit(char *indent) {
    char tmp = *indent;
    int index= 0;
    int converted = 0;
    while (tmp != '\0') {
        converted = converted * 10;
        if (tmp < 48 || tmp > 57)
            return -1;
        index = index + 1;
        tmp = tmp - 48;
        converted = converted + (int)tmp;
        tmp = *(indent + index);
    }
    // if (converted < 0 || converted > 255)
    //     return -1;
    //REMEBER LATER TO CHECK VALUES BETWEEN 127 AND 255 AS ITS DIFFERENCE BETWEEN SIGNED AND UNSIGNED
    return converted;
}

int validargs(int argc, char **argv) {
    // TO BE IMPLEMENTED
    if (argc == 1)
        return -1;
    char *t = *(argv + 1);
    if (cmp(t, "-h") == 0) {
        global_options |= 0x80000000;
        return 0;
    } else if (cmp(t, "-c") == 0) {
        if (argc > 2)
            t = *(argv + 2);
        if (argc == 2) {
            global_options |= 0x20000000;
            return 0;
        } else if (cmp(t, "-p") == 0) {
            if (argc == 4) {
                if (validDigit(*(argv + 3)) < 0)
                    return -1;
                global_options |= 0x30000000;
                global_options |= (validDigit(*(argv + 3)));
            } else if (argc > 4)
                return -1;
            if (argc == 3)
                global_options |= 0x30000004;
            return 0;
        }
    } else if (cmp(t, "-v") == 0) {
        if (argc != 2)
            return -1;
        global_options |= 0x40000000;
        return 0;
    }
    return -1;
}