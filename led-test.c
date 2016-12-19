#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#define FILE_NAME "/dev/simple-led"
#define MSG_LEN 1
#define BUF_SIZE (MSG_LEN + 1)

int main()
{
	int fd;
	char out_buffer[BUF_SIZE] = "";
	char in_buffer[BUF_SIZE] = "";

	fd = open(FILE_NAME, O_RDWR | O_SYNC);
	if (fd < 0) {
		perror("Failed to open device");
		exit(errno);
	}

	read(fd, in_buffer, MSG_LEN);
	printf("LED initial state: %s\n", in_buffer);

	if (!fgets(out_buffer, BUF_SIZE, stdin)) {
		printf("Could not read value. Exiting.\n");
		exit(-EIO);
	}

	// TODO: Check return values for read, write, close
	write(fd, out_buffer, MSG_LEN);
	
	read(fd, in_buffer, MSG_LEN);
	printf("LED final state: %s\n", in_buffer);
	
	close(fd);

	return EXIT_SUCCESS;
}
