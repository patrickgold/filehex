/*!filehex.c
* (c) 2018 Patrick Goldinger. All rights reserved.
* Source file for simple tool called 'filehex'.
*
* Licensed under the MIT license. See LICENSE file for more information.
* version: v.0.1.3.alpha
*/

#if defined(_WIN32) || defined(_WIN64)
#define __usingwindows__ 1
#define _CRT_SECURE_NO_WARNINGS // so fopen, sscanf, etc. works without _s
#elif defined(linux) || defined(__linux) || defined(__linux__)
#define __usinglinux__ 1
#else
#error "Unsupported OS-type."
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct filehex {
    char *filePath;
    unsigned bytesPerLine;
    unsigned long maxReadBytes;
    unsigned bool_showOffset;
    unsigned bool_editMode;
};

int process_option(struct filehex *, char *, char *);
int file_output_hex(struct filehex *);

int main(int argc, char **argv) {
    int i, ret_code;
    char *key, *value;
    struct filehex filehex_options = {
        NULL, 16, 0, 1, 0
    };

    if (argc < 2) {
        printf("Usage: %s [options] <filePath>\n", argv[0]);
        #if _DEBUG == 1 && defined(__usingwindows__)
        printf("Program ended with code 0.");
        getchar();
        #endif
        return 0;
    }

    // Solution based on this answer:
    // https://stackoverflow.com/a/12689505
    for (i = 1; i < argc; i++) {
        if (*argv[i] == '-') {
            key = argv[i] + 1;
            value = strchr(key, '=');
            if (value != NULL) *value++ = 0;
            ret_code = process_option(&filehex_options, key, value);
            if (ret_code != 0)
                break;
        }
        else {
            filehex_options.filePath = argv[i];
            file_output_hex(&filehex_options);
            break;
        }
    }

    #if _DEBUG == 1 && defined(__usingwindows__)
    printf("Program ended with code 0.");
    getchar();
    #endif
    return EXIT_SUCCESS;
}


int process_option(struct filehex *filehexopts, char *key, char *value) {
    long int tmp;
    if (strcmp(key, "?") == 0) {
        printf("filehex v.0.1.3.alpha\n");
        printf("(c) 2018 Patrick Goldinger. All rights reserved.\n\n");
        printf("usage: filehex [options] <filePath>\n");
        printf("e.g. : filehex -n=512 /home/user/test.txt\n\n");
        printf("  ?     shows this help.\n");
        printf("  n=    maximal length of read bytes.\n");
        printf("  o=    show(1) / hide(0) offset (Default is 1).\n");
        printf("  z=    bytes displayed per line.\n");
    }
    else if (strcmp(key, "n") == 0) {
        sscanf(value, "%ld", &tmp);
        if (tmp < 0) {
            fprintf(stderr, "The argument value given for parameter 'n' is invalid.\n");
            return -1;
        }
        filehexopts->maxReadBytes = tmp;
    }
    else if (strcmp(key, "o") == 0) {
        sscanf(value, "%ld", &tmp);
        if (tmp < 0 || tmp > 1) {
            fprintf(stderr, "The argument value given for parameter 'o' is invalid.\n");
            return -1;
        }
        filehexopts->bool_showOffset = tmp;
    }
    else if (strcmp(key, "z") == 0) {
        sscanf(value, "%ld", &tmp);
        if (tmp <= 0) {
            fprintf(stderr, "The argument value given for parameter 'z' is invalid.\n");
            return -1;
        }
        filehexopts->bytesPerLine = tmp;
    }
    else if (strcmp(key, "e") == 0) {
        filehexopts->bool_editMode = 1;
    }
    return 0;
}


/// <summary>Outputs a file in hexadecimal representation.
/// Use filehex struct to set options on behaviour and layout.
/// Returns 0 if success, -1 when buffer allocating fails or
/// a value greater 0 (errno) when fopen is NULL.</summary>
/// <param name='filehexopts'>Preferences struct.</param>
int file_output_hex(struct filehex *filehexopts) {

    if (filehexopts->bool_editMode == 1)
        puts("Edit mode currently not supported!");

    // define variables used.
    unsigned 
        n, m, // counter vars
        offset = 0, // sum of bytes read
        eof_reached = 0; // bool-like int for while loop
    size_t 
        bytes_read = 0, // holds the bytes read of fread()
        bytesToRead = 0; // holds how many bytes have to be read

    // allocate buffer with size of filehexopts->bytesPerLine
    unsigned char *buffer = (unsigned char *)malloc(filehexopts->bytesPerLine);
    if (buffer == NULL) {
        fprintf(stderr, "Something went wrong while allocating the buffer...\n");
        return -1;
    }

    // open file
    FILE *fp = fopen(filehexopts->filePath, "rb");
    if (fp == NULL) {
        fprintf(stderr, "'%s': %s\n", filehexopts->filePath, strerror(errno));
        return errno;
    }

    // main loop, which runs until end of file reached.
    while (!eof_reached) {
        // check out how many bytes have to be read
        if (filehexopts->maxReadBytes > 0)
            bytesToRead = 
                filehexopts->maxReadBytes - offset > filehexopts->bytesPerLine 
                ? filehexopts->bytesPerLine 
                : filehexopts->maxReadBytes - offset;
        else
            bytesToRead = filehexopts->bytesPerLine;
        // read bytes
        bytes_read = fread(buffer, sizeof(unsigned char), bytesToRead, fp);
        // if no bytes were read, file is EOF
        if (bytes_read == 0)
            break;
        // show offset
        if (filehexopts->bool_showOffset == 1)
            printf("%08x  ", offset);
        // loop for hexadecimal representation
        for (n = 0; n < bytes_read; n++) {
            buffer[n] &= 0xFF; // make sure that higer bytes are nulled.
            printf("%02x ", (unsigned int)buffer[n]);
            if (n + 1 == bytes_read && bytes_read < filehexopts->bytesPerLine) {
                eof_reached = 1;
                for (m = n + 1; m < filehexopts->bytesPerLine; m++)
                    printf("   ");
            }
        }
        putchar(' ');
        // loop for ascii output
        for (n = 0; n < bytes_read; n++) {
            putchar(buffer[n] < 32 ? '.' : buffer[n]);
            if (n + 1 == bytes_read && bytes_read < filehexopts->bytesPerLine) {
                eof_reached = 1;
                for (m = n + 1; m < filehexopts->bytesPerLine; m++)
                    putchar(' ');
            }
        }
        offset += bytes_read; // increase offset by bytes read.
        putchar('\n');
    }

    // free allocated momory for buffer
    free(buffer);
    // close file
    fclose(fp);

    // return 0 -> success
    return 0;
}
