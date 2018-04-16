/*!filehex.c
* (c) 2018 Patrick Goldinger. All rights reserved.
* Source file for simple tool called 'filehex'.
*
* Licensed under the MIT license. See LICENSE file for more information.
* version: v.0.1.1.alpha
*/

#if defined(_WIN32) || defined(_WIN64)
#define __usingwindows__
#define _CRT_SECURE_NO_WARNINGS // so fopen, sscanf, etc. works without _s
#elif defined(linux) || defined(__linux) || defined(__linux__)
#define __usinglinux__
#else
#error "Unsupported OS-type."
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct filehex {
	char *filePath;
	unsigned bytesPerLine;
	unsigned long maxReadBytes;
	unsigned bool_showOffset;
};

int process_option(struct filehex *, char *, char *);
int file_output_hex(struct filehex *);

int main(int argc, char **argv) {
	int i, ret_code;
	char *key, *value;
	struct filehex filehex_options = {
		NULL, 16, 0, 1
	};

	if (argc < 2) {
		printf("Usage: %s [options] <filePath>\n", argv[0]);
#if _DEBUG == 1
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

#if _DEBUG == 1
	getchar();
#endif
	return EXIT_SUCCESS;
}

int process_option(struct filehex *filehexopts, char *key, char *value) {
	long int tmp;
	if (strcmp(key, "?") == 0) {
		printf("filehex v.0.1.1.alpha\n");
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
	return 0;
}

int file_output_hex(struct filehex *filehexopts) {
	int tmp = 0, eof_reached = 0, n, m, offset = 0;
	long fpSize;
	size_t bytes_read = 0;
	unsigned char *buffer = (unsigned char *)malloc(filehexopts->bytesPerLine);
	if (buffer == NULL) {
		fprintf(stderr, "Something went wrong while allocating the buffer...\n");
		return -1;
	}
	FILE *fp = fopen(filehexopts->filePath, "rb");
	if (fp == NULL) {
		fprintf(stderr, "'%s': %s\n", filehexopts->filePath, strerror(errno));
		return errno;
	}
	fseek(fp, 0, SEEK_END);
	fpSize = ftell(fp);
	rewind(fp);
	if (filehexopts->maxReadBytes > 0) {
		fpSize = filehexopts->maxReadBytes >= fpSize ? fpSize : filehexopts->maxReadBytes;
	}
	//printf("OUTPUT FILE: '%s'\n\n", filehexopts->filePath);
	while (!eof_reached) {
		if (filehexopts->bool_showOffset == 1)
			printf("%08x  ", offset);
		bytes_read = fread(buffer, sizeof(unsigned char), fpSize >= filehexopts->bytesPerLine ? filehexopts->bytesPerLine : fpSize, fp);
		for (n = 0; n < filehexopts->bytesPerLine; n++) {
			buffer[n] &= 0xFF;
			printf("%02x ", (unsigned int)buffer[n]);
			if (n + 1 == fpSize) {
				eof_reached = 1;
				for (m = n + 1; m < filehexopts->bytesPerLine; m++)
					printf("   ");
				break;
			}
		}
		printf(" ");
		for (n = 0; n < filehexopts->bytesPerLine; n++) {
			printf("%c", buffer[n] < 32 ? '.' : buffer[n]);
			if (n + 1 == fpSize) {
				eof_reached = 1;
				for (m = n + 1; m < filehexopts->bytesPerLine; m++)
					printf(" ");
				break;
			}
		}
		fpSize -= filehexopts->bytesPerLine;
		offset += filehexopts->bytesPerLine;
		printf("\n");
	}
	free(buffer);
	return 0;
}
