#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "coreinutils.h"

#define PROG_NAME "test"

void usage(int argc, char** argv) {
    fprintf(stderr,
            "coreinutils %s %s\n"
            "Usage:\n"
            "\n"
            "[ EXPRESSION ]\n"
            "or\n"
            "test EXPRESSION\n"
            "Use '%s --help' for help\n",
            PROG_NAME, VERSION, argv[0]
            );

    exit(EXIT_FAILURE);
}

void print_help(int argc, char** argv);
void print_tests(void);

void test_str_equal(char* str1, char* str2, uint8_t mode);
void test_str_lenght(char* str, uint8_t mode);
void test_int(char* int1, char* int2, uint8_t mode);
void test_file_exist(const char* filename);

int8_t EXIT_CODE = -1;
uint8_t TEST_MODE = 0;

int main(int argc, char** argv) {
    if (argc == 1)
        usage(argc, argv);

    if (!strcmp("--help", argv[1]))
        print_help(argc, argv);
    else if (!strcmp("--tests", argv[1]))
        print_tests();

    int lchar;
    for (; *argv[0]; argv[0]++)
        lchar = *argv[0];

    if (lchar == '[') {
        if ((argc == 5) && (!strcmp("]", argv[4])))
            TEST_MODE = 1;
        else if ((argc == 4) && (!strcmp("]", argv[3])))
            TEST_MODE = 2;
        else
            usage(argc, argv);
    } else {
        if (argc == 4)
            TEST_MODE = 1;
        else if (argc == 3)
            TEST_MODE = 2;
        else
            usage(argc, argv);
    }

    if (TEST_MODE == 1) {
        if (!strcmp("=", argv[2]))
            test_str_equal(argv[1], argv[3], 0);
        else if (!strcmp("!=", argv[2]))
            test_str_equal(argv[1], argv[3], 1);
        else if (!strcmp("-eq", argv[2]))
            test_int(argv[1], argv[3], 0);
        else if (!strcmp("-ge", argv[2]))
            test_int(argv[1], argv[3], 1);
        else if (!strcmp("-gt", argv[2]))
            test_int(argv[1], argv[3], 2);
        else if (!strcmp("-le", argv[2]))
            test_int(argv[1], argv[3], 3);
        else if (!strcmp("-lt", argv[2]))
            test_int(argv[1], argv[3], 4);
        else if (!strcmp("-ne", argv[2]))
            test_int(argv[1], argv[3], 5);
        else
            print_tests();
    }

    else if (TEST_MODE == 2) {
        if (!strcmp("-e", argv[1]))
            test_file_exist(argv[2]);
        else if (!strcmp("-n", argv[1]))
            test_str_lenght(argv[2], 0);
        else if (!strcmp("-z", argv[1]))
            test_str_lenght(argv[2], 1);
        else
            print_tests();
    }

    exit(EXIT_CODE);
}

void test_str_equal(char* str1, char* str2, uint8_t mode) {
    if (!strcmp(str1, str2)) {
        if (!mode)
            EXIT_CODE = 0;
        else if (mode)
            EXIT_CODE = 1;
    } else {
        if (mode)
            EXIT_CODE = 1;
        else if (!mode)
            EXIT_CODE = 0;
    }

    return;
}

void test_str_lenght(char* str, uint8_t mode) {
    if (strlen(str)) {
        if (!mode)
            EXIT_CODE = 0;
        else if (mode)
            EXIT_CODE = 1;
    } else {
        if (mode)
            EXIT_CODE = 0;
        else if (!mode)
            EXIT_CODE = 1;
    }

    return;
}

void test_int(char* int1, char* int2, uint8_t mode) {
    int i1 = atoi(int1);
    int i2 = atoi(int2);

    if (!mode) {
        if (i1 == i2)
            EXIT_CODE = 0;
        else
            EXIT_CODE = 1;
    } else if (mode == 1) {
        if (i1 >= i2)
            EXIT_CODE = 0;
        else
            EXIT_CODE = 1;
    } else if (mode == 2) {
        if (i1 > i2)
            EXIT_CODE = 0;
        else
            EXIT_CODE = 1;
    } else if (mode == 3) {
        if (i1 <= i2)
            EXIT_CODE = 0;
        else
            EXIT_CODE = 1;
    } else if (mode == 4) {
        if (i1 < i2)
            EXIT_CODE = 0;
        else
            EXIT_CODE = 1;
    } else if (mode == 5) {
        if (i1 != i2)
            EXIT_CODE = 0;
        else
            EXIT_CODE = 1;
    }

    return;
}

void test_file_exist(const char* filename) {
    if (fopen(filename, "rb") == NULL)
        EXIT_CODE = 1;
    else
        EXIT_CODE = 0;
}

void print_help(int argc, char** argv) {
    fprintf(stderr,
            "coreinutils test %s\n"
            "\n"
            "test - Exit with the status determined by EXPRESSION.\n"
            "Usage:\n"
            "[ EXPRESSION ]\n"
            "or\n"
            "test EXPRESSION\n"
            "\n"
            "Exit codes:\n"
            "-1: default\n"
            "0: true\n"
            "1: false\n"
            "\n"
            "Expected use:\n"
            "[ EXPRESSION1 TEST EXPRESSION2 ] - string/integer tests\n"
            "OR\n"
            "[ TEST FILE ] - file tests\n"
            "\n"
            "Use'%s --tests' to show all tests\n",
            VERSION, argv[0]
            );

    exit(EXIT_SUCCESS);
}

void print_tests(void) {
    fputs(
        "coreinutils test\n"
        "List of string/integer tests:\n"
        "\n"
        "   -n STRING\n"
        "       the lenght of STRING is nonzero\n"
        "   -z STRING\n"
        "       the lenght of STRING is zero\n"
        "   STRING1 = STRING2\n"
        "       check if STRING1 is equal to STRING2\n"
        "   STRING1 != STRING2\n"
        "       check if STRING1 is not equal to STRING2\n"
        "   INT1 -eq INT2\n"
        "       INT1 is equal to INT2\n"
        "   INT1 -ge INT2\n"
        "       INT1 is greater than or equal to INT2\n"
        "   INT1 -gt INT2\n"
        "       INT1 is greater than INT2\n"
        "   INT1 -le INT2\n"
        "       INT1 is less than or equal to INT2\n"
        "   INT1 -lt INT2\n"
        "       INT1 is less than INT2\n"
        "   INT1 -ne INT2\n"
        "       INT1 is not equal to INT2\n"
        "\n"
        "List of file tests:\n"
        "   -e FILE\n"
        "       check if FILE exists\n",
        stderr
        );

    exit(EXIT_FAILURE);
}
