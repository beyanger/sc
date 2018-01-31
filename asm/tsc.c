
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>


uint64_t rte_rdtsc(void)
{
	union {
		uint64_t tsc_64;
		struct {
			uint32_t lo_32;
			uint32_t hi_32;
		};
	} tsc;

	asm volatile("rdtsc":
			"=a"(tsc.lo_32),
			"=d"(tsc.hi_32));
	return tsc.tsc_64;
}

int main() 
{
	uint64_t start, end;
	start = rte_rdtsc();
	__int128 a = 3, b = 499;
	usleep(1e6);
	end = rte_rdtsc();
	printf("%f MHz\n", (end-start)/(1024.*1024));
	printf("%lu\n", end/((end-start)*3600));
	printf("%jd\n", a+b);
	return 0;
}

