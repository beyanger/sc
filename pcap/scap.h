
#ifndef __SCAP_H
#define __SCAP_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>


typedef struct scap {
	int fd;
} scap_t;


scap_t *scap_open(const char *path, mode_t mode);


void scap_append(scap_t *st, const char *bytes, const int len, struct timeval tv);


void scap_close(scap_t *st);

#endif

