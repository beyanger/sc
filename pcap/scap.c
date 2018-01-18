
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "scap.h"


struct pcap_file_header {
	uint32_t magic;
	uint16_t major;
	uint16_t minor;
	int32_t  zone;
	uint32_t sigflags;
	uint32_t snaplen;
	uint32_t linktype;
} __attribute__((packed));


struct packet_header {
	uint32_t sec;
	uint32_t msec;
	uint32_t caplen;
	uint32_t len;
} __attribute__((packed));



scap_t *scap_open(const char *path, mode_t mode)
{
	struct pcap_file_header h;
	scap_t *t;
	ssize_t ret;
	int fd = open(path, O_CREAT|O_WRONLY , mode);

	if (fd == -1) 
		return NULL;

	t = (scap_t *)malloc(sizeof(scap_t));

	if (t == NULL)
		return NULL;

	t->fd = fd;

	h.magic = htonl(0xd4c3b2a1);
	h.major = htons(0x0200);
	h.minor = htons(0x0400);
	h.zone = 0;
	h.sigflags = 0;
	h.snaplen = htonl(0xffff0000);
	h.linktype = 1;
	
	ret = write(fd, &h, sizeof(h));

	if (ret != sizeof(h)) {
		free(t);
		return NULL;
	}
	return t;
}



void scap_append(scap_t *st, const char *bytes, const int len, struct timeval tv) {
	if (!st)
		return;

	struct packet_header h;
	h.sec = (uint32_t)tv.tv_sec;
	h.msec = (uint32_t)tv.tv_usec/1000;
	h.caplen = h.len = len;

	write(st->fd, &h, sizeof(h));
	write(st->fd, bytes, len);
}


void scap_close(scap_t *st)
{
	if (st) {
		close(st->fd);
		free(st);
	}
}

