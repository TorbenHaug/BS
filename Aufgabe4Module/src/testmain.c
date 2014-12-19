#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
	char *msg = "Test Message";
	int size = strlen(msg);
	int file_handle = open("/dev/trans0", O_RDWR);
	ssize_t result = write(file_handle, msg, size);
	char *enc_msg = malloc(sizeof(char) * size);
	read(file_handle, enc_msg, size);
	printf("%s -> %s\n", msg, enc_msg);
	close(file_handle);
	file_handle = open("/dev/trans1", O_RDWR);
	result = write(file_handle, enc_msg, size);
	char *dec_msg = malloc(sizeof(char) * size);
	read(file_handle, dec_msg, size);
	printf("%s -> %s\n", enc_msg, dec_msg);
	close(file_handle);
	return 0;
}
