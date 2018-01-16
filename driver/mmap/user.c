
#include <stdio.h>  
#include <fcntl.h>  
#include <sys/mman.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>


int main(int argc, char **argv)
{
	int fd;
	char *map_buf;

	fd = open("/dev/misc", O_RDWR);
	if (fd == -1) {
		perror("open:");
		return -1;
	}

	map_buf = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

	if (argv[1][0] == 'r') {
		printf("print: %s\n", map_buf);
	} else {
		strcpy(map_buf, argv[2]);	
	}

	munmap(map_buf, 1024);
	close(fd);
	return 0;
}
 
