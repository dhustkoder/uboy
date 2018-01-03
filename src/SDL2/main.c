#include <stdio.h>
#include <stdlib.h>
#include "log.h"
#include "cpu.h"
#include "rom.h"


static uint8_t* readfile(const char* const filename)
{
	FILE* const file = fopen(filename, "r");
	if (file == NULL) {
		logerror("Couldn't open file.\n");
		return NULL;
	}

	uint8_t* data = NULL;

	fseek(file, 0, SEEK_END);
	const long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (size < 0x4000) {
		logerror("rom size is too small.\n");
		goto Lfclose;
	}

	data = malloc(size);

	if (data == NULL) {
		logerror("Couldn't allocate memory.\n");
		goto Lfclose;
	}

	if (fread(data, 1, size, file) < (unsigned long)size) {
		logerror("Couldn't read file.\n");
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

	int ret = EXIT_SUCCESS;
	
	if (!loadrom(data)) {
		ret = EXIT_FAILURE;
		goto Lfreedata;
	}

	resetcpu();

	for (;;) {
		stepcpu();
		printcpu();
		getc(stdin);
	}

	unloadrom();
Lfreedata:
	free(data);
	return ret;
}

