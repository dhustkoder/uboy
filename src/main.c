#include <stdio.h>
#include <stdlib.h>
#include "rom.h"


static uint8_t* readfile(const char* const filename)
{
	FILE* const file = fopen(filename, "r");
	if (file == NULL) {
		perror("Couldn't open file: ");
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	const long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	uint8_t* data = malloc(size);

	if (data == NULL) {
		perror("Couldn't allocate memory: ");
		goto Lfclose;
	}

	if (fread(data, 1, size, file) < (unsigned long)size) {
		perror("Couldn't read file: ");
		free(data);
		data = NULL;
	}

Lfclose:
	fclose(file);

	return data;
}


int main(const int argc, const char* const * const argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [romfile]\n", argv[0]);
		return EXIT_FAILURE;
	}

	uint8_t* const data = readfile(argv[1]);
	if (data == NULL)
		return EXIT_FAILURE;


	int ret = EXIT_FAILURE;
	if (!loadrom(data)) {
		fprintf(stderr, "Couldn't load rom\n");
		goto Lfreedata;
	}

	freerom();
	ret = EXIT_SUCCESS;

Lfreedata:
	free(data);
	return ret;
}

