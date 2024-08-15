#include <stdio.h>
#include "parser.h"

int main (int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Not enough arguments\n");
		return 1;
	}
	
	FILE *file_ptr = fopen(argv[1], "r");
	
	parser(file_ptr);
	
	fclose(file_ptr);
	
	return 0;
}