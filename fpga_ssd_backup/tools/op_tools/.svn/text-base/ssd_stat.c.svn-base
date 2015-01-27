#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#define BUFSIZE 1024
#define ARRAY 45
#define MAXVALUE ((unsigned long)1 << 32 - 1)
#define PAGES 128
#define BLOCKS 8000
#define TOTALBLOCKS ((ARRAY - 1) * BLOCKS)

enum TYPE {
	READ = 0,
	WRITE = 1,
	DELETE = 2,
	FREE = 3,
	ALL = 4,
};
typedef unsigned int u32_t;

char *proc_path = "/proc/Baidu_ssd_info/speed";
//char *proc_path = "speed";

static int getvalue(u32_t array[][ALL] , u32_t size)
{
	char buf[BUFSIZE] = {'\0'};
	FILE *fp = fopen(proc_path, "r");
	char *token = NULL;
	char *values[10];
	int filed, i;
	char *ptr = NULL;
	char *pbuf = NULL;

	if(!fp)
	{
		printf("read proc error\n");
		return -1;
	}

	while(fgets(buf, BUFSIZE, fp) && size--) {
		pbuf = buf;
		filed = 0;
		while(NULL != (token = strtok_r(pbuf, " ", &ptr))) {
			pbuf = NULL;
			values[filed++] = token;
		}

		sscanf(values[2], "%u", &array[ARRAY - 1 - size][READ]);
		sscanf(values[4], "%u", &array[ARRAY - 1 - size][WRITE]);
		sscanf(values[6], "%u", &array[ARRAY - 1 - size][DELETE]);
		sscanf(values[8], "%u", &array[ARRAY - 1 - size][FREE]);

		memset(buf, 0, BUFSIZE);
	}

	fclose(fp);
	return 0;
}

static void calcvalue(u32_t arr1[][ALL], u32_t arr2[][ALL], int size)
{
	int i = 0;

	for(; i < size; i++) {
		arr1[i][READ] = (arr2[i][READ] + MAXVALUE - arr1[i][READ]) % MAXVALUE;
		arr1[i][WRITE] = (arr2[i][WRITE] + MAXVALUE - arr1[i][WRITE]) % MAXVALUE;
		arr1[i][DELETE] = (arr2[i][DELETE] + MAXVALUE - arr1[i][DELETE]) % MAXVALUE;
		arr1[i][FREE] = arr2[i][FREE];
	}
}

static void printsingle(u32_t array[][ALL], int channel, int interval, int page)
{
	if(page)
		printf("Channel[%d]:\t %u\t\t %u\t\t %u\t\t %u\t\t %d\n", channel, 
				array[channel + 1][READ] * 8 / interval, 
				array[channel + 1][WRITE] / PAGES / interval, 
				array[channel + 1][DELETE] / PAGES / interval,
				array[channel + 1][FREE] * 2,	
				100 - (100 * array[channel + 1][FREE]) / BLOCKS);	
	else
		printf("Channel[%d]:\t %u\t\t %u\t\t %u\t\t %u\t\t %d\n", channel, 
				array[channel + 1][READ] / PAGES / interval, 
				array[channel + 1][WRITE] / PAGES / interval, 
				array[channel + 1][DELETE] / PAGES / interval,
				array[channel + 1][FREE] * 2,	
				100 - (100 * array[channel + 1][FREE]) / BLOCKS);	
		

}

static void printtotal(u32_t array[][ALL], int interval, int page)
{
	if(page)
		printf("Total:\t\t %u\t\t %u\t\t %u\t\t %u\t\t %d\n", 
				array[0][READ] * 8/ interval, 
				array[0][WRITE] / PAGES / interval, 
				array[0][DELETE] / PAGES / interval,
				array[0][FREE] * 2,
				100 - (100 * array[0][FREE]) / TOTALBLOCKS);	
	else
		printf("Total:\t\t %u\t\t %u\t\t %u\t\t %u\t\t %d\n", 
				array[0][READ] / PAGES / interval, 
				array[0][WRITE] / PAGES / interval, 
				array[0][DELETE] / PAGES / interval,
				array[0][FREE] * 2,	
				100 - (100 * array[0][FREE]) / TOTALBLOCKS);	

}

static void printinfo(u32_t array[][ALL], int size, int interval, int page)
{
	int i = 0;

	for(; i < size - 1; i++) {
		printsingle(array, i, interval, page);	
	}

	printf("\n\n");

}

int main(int argc, char *argv[])
{
	u32_t array_prev[ARRAY][ALL];
	u32_t array_next[ARRAY][ALL];
	int c;
	int interval = 1;
	int times = 0;
	int channel = -1;
	int total = 0;
	int page = 0;

	while((c = getopt(argc, argv, "i:c:spxh")) > 0) {
		switch(c) {
			case 'h':
				printf("%s -i<interval> -c<channel> -s<single total value> -p<page> -x\n", argv[0]);
				return 0;
			case 'i':
				interval = atoi(optarg);
				if(interval <= 0) {
					printf("error interval: %d [>0]\n", interval);
					return -1;
				}
				break;
			case 'c':
				channel = atoi(optarg);
				if(channel < 0 || channel > 43) {
					printf("error channel: %d [0-43]\n", channel);
					return -1;
				}
				break;
			case 's':
				total = 1;
				break;
			case 'p':
				page = 1;
				break;
			case 'x':
				times = 1;
				break;
			defalt:
				printf("error arg\n");
				return -1;
		}
	}
	
	while(1) {
		if(getvalue(array_prev, ARRAY) < 0)
			return -1;
		sleep(interval);
		if(getvalue(array_next, ARRAY) < 0)
			return -1;

		calcvalue(array_prev, array_next, ARRAY);

		printf("----------------ssd info-----------------\n");

		if(page)
			printf("Channel  \tREAD(KB/s)\tWRITE(MB/s)\tDELETE(MB/s)\tFREE(MB)\tUSED(%)\n");
		else
			printf("Channel  \tREAD(MB/s)\tWRITE(MB/s)\tDELETE(MB/s)\tFREE(MB)\tUSED(%)\n");

		if(total) {
			printtotal(array_prev, interval, page);
			if(channel >= 0)
				printsingle(array_prev, channel, interval, page);
		} else {
			if(channel >= 0)
				printsingle(array_prev, channel, interval, page);
			else {
				printtotal(array_prev, interval, page);
				printinfo(array_prev, ARRAY, interval, page);	
			}
		}

		if(!times)
			break;
	}

	return 0;
}
