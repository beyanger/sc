
#include <sys/mman.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


#define UIO_DEV		"/dev/uio0"
#define UIO_ADDR	"/sys/class/uio/uio0/maps/map0/addr"
#define UIO_SIZE	"/sys/class/uio/uio0/maps/map0/size"



int main(int argc, char **argv) {
	int uio_fd, addr_fd, size_fd;
	long uio_size;
	char *uio_addr, *access_addr;
	char addr_buf[128], size_buf[128];


	uio_fd = open(UIO_DEV, O_RDWR);
	addr_fd = open(UIO_ADDR, O_RDONLY);
	size_fd = open(UIO_SIZE, O_RDONLY);

	if (uio_fd == -1 || addr_fd == -1 || size_fd == -1) {
		perror("open:");
		return -1;
	}

	read(addr_fd, addr_buf, sizeof(addr_buf));
	read(size_fd, size_buf, sizeof(size_buf));

	uio_addr = (char *)strtoul(addr_buf, NULL, 0);
	uio_size = (long)strtoul(size_buf, NULL, 0);


	printf("uio at: %p, size: %lu\n", uio_addr, uio_size);


	access_addr = mmap(NULL, uio_size, PROT_READ|PROT_WRITE, MAP_SHARED, uio_fd, 0);

	sprintf(access_addr, "uio at: %p, size %lu, user: %s\n", uio_addr, uio_size, argv[1]);

	munmap(access_addr, uio_size);
	close(uio_fd);
	close(addr_fd);
	close(size_fd);
	return 0;
}




