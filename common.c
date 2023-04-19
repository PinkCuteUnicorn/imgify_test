#include "common.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

size_t fsize(const char *filename) {
	struct stat stat_buf;
	if (stat(filename, &stat_buf) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}
	return (size_t) stat_buf.st_size;
}
