#include <unistd.h>
#include <time.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <linux/if_tun.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <stdlib.h>

int tun_alloc(int flags) {
	struct ifreq ifr;
	int fd, err;

	if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
		perror("open");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags |= flags;
	err = ioctl(fd, TUNSETIFF, (void *)&ifr);
	if (err < 0) {
		close(fd);
		return err;
	}

	printf("Open tun device: %s for reading...\n", ifr.ifr_name);
	return fd;
}

static unsigned short checksum(unsigned short *data, int size) {
	unsigned origsum = 0;
	while (size > 1) {
		origsum += *data++;
		size -= 2;
	}
	if (size) 
		origsum += ((*(unsigned char *)data) & 0xff) << 8;

	origsum = (origsum & 0xffff) + (origsum >> 16);
	origsum = (origsum & 0xffff) + (origsum >> 16);

	return (~origsum & 0xffff);
}



int main() {

	int tfd, nread;
	char buffer[1024];

	tfd = tun_alloc(IFF_TUN|IFF_NO_PI);

	if (tfd < 0) {
		perror("alloc interface");
		return -1;
	}

	srand(time(NULL));
	unsigned short id = random() & 0xffff;

	while (1) {
		nread = read(tfd, buffer, sizeof(buffer));
		if (nread < 0) {
			perror("read error");
			return -1;
		}

		struct iphdr *iph = (struct iphdr *)buffer;
		struct icmphdr *ich = (struct icmphdr *)(buffer + iph->ihl*4);

		unsigned t = iph->saddr;
		iph->saddr = iph->daddr;
		iph->daddr = t;
		iph->id = id++;
		iph->check = 0;
		iph->check = checksum((unsigned short *)iph, iph->ihl*4);


		ich->type = ICMP_ECHOREPLY;
		ich->checksum = 0;
		ich->checksum = checksum((unsigned short *)ich, nread-iph->ihl*4);

		sleep(1);
		write(tfd, buffer, nread);

	}
	return 0;
}



