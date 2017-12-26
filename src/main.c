#include <stdio.h>
#include <stdlib.h>
#include "rom.h"


int main(const int argc, const char* const * const argv)
{
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [romfile]\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (!loadrom(argv[1]))
		return EXIT_FAILURE;

	freerom();
	return EXIT_SUCCESS;
}

