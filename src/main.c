#include <stdio.h>
#include <stdlib.h>
#include "rom.h"


int main(int argc, char** argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [romfile]\n", argv[0]);
		return EXIT_FAILURE;
	}

	FILE* const file = fopen(argv[1], "r");
	if (file == NULL) {
		perror("Couldn't open file: ");
		return EXIT_FAILURE;
	}

	fseek(file, 0, SEEK_END);
	const long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	uint8_t* const data = malloc(size);
	int ret = EXIT_FAILURE;

	if (data == NULL) {
		perror("Couldn't allocate memory: ");
		goto Lfclose;
	}

	if (fread(data, 1, size, file) < (unsigned long)size) {
		perror("Couldn't read file: ");
		goto Lfreedata;
	}

	if (!loadrom(data)) {
		fprintf(stderr, "Couldn't load rom\n");
		goto Lfreedata;
	}

	freerom();
	ret = EXIT_SUCCESS;

Lfreedata:
	free(data);
Lfclose:
	fclose(file);
	return ret;
}

