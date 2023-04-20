
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define PROGRAM "functionalTest"
#define VERSION "1.0"

#include "imgify.h"
#include "common.h"
#include "common_options.h"


// Каст bin к png
static bool bin2pngTest(const options_t *options, void *data, size_t filesize) {
	const uint8_t channels = 4;
	const uint32_t width = (uint32_t) ceil(sqrt(filesize / channels));
	const uint32_t height = (uint32_t) ceil(filesize / channels / width) + 1;
	const uint32_t padding = (uint32_t) (width * height * channels - filesize);

	return png_save(options->output, data, width, height, channels, padding, options->pad_byte);
}

// Каст png к bin
static bool png2binTest(const options_t *options, void *data, size_t filesize) {
	uint8_t *buffer;
	size_t buffer_size;
	uint32_t width;
	uint32_t height;
	uint8_t channels;
	uint32_t padding;

	const bool ok = png_load(options->input, &buffer, &buffer_size, &width, &height, &channels, &padding, options->pad_byte);

	if (ok) {

		FILE *outf = fopen(options->output, "wb");
		if (outf == NULL) {
			perror("fopen");
			exit(EXIT_FAILURE);
		}

		size_t block_size = 8192;
		size_t remaining = buffer_size;
		size_t total_written = 0;

		while (remaining > 0) {
			if (remaining < block_size)
				block_size = remaining;
			size_t written = fwrite(buffer + total_written, 1, block_size, outf);
			if (written == 0)
				break;
			remaining -= written;
			total_written += written;
		}

		fclose(outf);
	}
	return ok;
}


int runPngTest(options_t *options){


	int fd = -1;

	if (options->input == NULL || options->output == NULL) {
		usage();
		goto failed;
	}

	//printf("Start png, input: %s\n", options->input);
	fd = open(options->input, O_RDONLY);
	if (fd == -1) {
		perror("open");
		goto failed;
	}

	const size_t filesize = fsize(options->input);
	//printf("Filesize png, input: %s, %i\n", options->input, filesize);

	void *data = mmap(0, filesize, PROT_READ, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		perror("mmap");
		goto failed;
	}

	close(fd);
	fd = -1;

	if(png2binTest(options, data, filesize)){
		//printf("Sucess png for input: %s\n", options->input);
	}
	else{
		printf("Error png for input: %s\n", options->input);
		goto failed;	
	}

	int ret = munmap(data, filesize);
	if (ret == -1) {
		perror("munmap");
		goto failed;
	}

	return EXIT_SUCCESS;
failed:
	if (fd != -1)
		close(fd);
	exit(EXIT_FAILURE);
}

int runBinTest(options_t *options){


	int fd = -1;

	if (options->input == NULL || options->output == NULL) {
		usage();
		goto failed;
	}
	
	//printf("Start bin, input: %s\n", options->input);
	fd = open(options->input, O_RDONLY);
	if (fd == -1) {
		perror("open");
		goto failed;
	}

	const size_t filesize = fsize(options->input);
	//printf("Filesize bin, input: %s, %i\n", options->input, filesize);

	void *data = mmap(0, filesize, PROT_READ, MAP_SHARED, fd, 0);
	if (data == MAP_FAILED) {
		perror("mmap");
		goto failed;
	}

	close(fd);
	fd = -1;

	if(bin2pngTest(options, data, filesize)){
		//printf("Sucess bin for input: %s\n", options->input);
	}
	else{
		printf("Error bin for input: %s\n", options->input);
		goto failed;	
	}

	int ret = munmap(data, filesize);
	if (ret == -1) {
		perror("munmap");
		goto failed;
	}

	return EXIT_SUCCESS;
failed:
	if (fd != -1)
		close(fd);
	exit(EXIT_FAILURE);
}



// Тест супер простой
// Ожидается поступление png, он трансформируется в bin и обратно. Происходит сравнение размера итогового png файла.
int runTest(options_t *options){

	
	int res = runPngTest(options);

	char* tmpInput = options->input;
	options->input = options->output;
	options->output = tmpInput;
	res = runBinTest(options);
	return res;
}

void my_itoa(int num, char *str)
{
    sprintf(str, "%d", num);
}


void _fillOptions_(uint32_t index, const char* dir, options_t *options){

	options->input = malloc(100);
	options->output = malloc(100);

	char intString[15]; 
	my_itoa(index, intString);

	memcpy(options->input, dir, strlen(dir));
	memcpy(options->input + strlen(dir), intString, strlen(intString));
	memcpy(options->input + strlen(dir) + strlen(intString), ".png", 4);
	memcpy(options->input + strlen(dir) + strlen(intString) + 4, "\0", 1);

	memcpy(options->output, dir, strlen(dir));
	memcpy(options->output + strlen(dir), intString, strlen(intString));
	memcpy(options->output + strlen(dir) + strlen(intString), ".bin", 4);
	memcpy(options->output + strlen(dir) + strlen(intString) + 4, "\0", 1);

}
void _clearOptions_(options_t *options){
	free(options->input);
	free(options->output);	
}


int main() {

	int result = 0;


	options_t *options = malloc(sizeof(options_t));
	if (options == NULL) {
		perror("malloc");
		exit(EXIT_FAILURE);
	}
	memset(options, 0, sizeof(options_t));


	for (int i = 0; i < 15; i++){

		_fillOptions_(i, "resources/", options);
		//printf("Start input %s, output %s\n", options->input, options->output);
		if (runTest(options) != 0){
			result = EXIT_FAILURE;
		}
		//printf("End input %s, output %s\n", options->input, options->output);
		_clearOptions_(options);
	}


	if (options != NULL)
		free(options);

	return result;
}
