#include "ssd_api.h"

#define SIZE (10 * 1024 * 1024)
/*for crc*/
unsigned long long table[256];
unsigned long long key;

#define CRC_DATA 0x1LL
#define CRC_POLY 0x11edc6f41LL
#define CRC_DATA_BIT (1024 * 8)
#define CRC_POLY_BIT 33
#define CRC_BLOCK_HEADER  32

struct BlockId argv2id(char *str)
{
	struct BlockId id;
	int i;
	char ch;
	unsigned long long sum = 0;

	for(i = 0; i < 32; i++) {
		if(16 == i) {
			id.m_nHigh = sum;
			sum = 0;
		}
		ch = *(str + i);
		if(ch >= '0' && ch <= '9')
			sum = sum * 16 + ch - '0';
		else if(ch >= 'A' && ch <= 'F')
			sum = sum * 16 + ch - 'A' + 10;
		else
			sum = sum * 16 + ch - 'a' + 10;
	}

	id.m_nLow = sum;
//	printf("%llu\n %llu\n", id.m_nHigh, id.m_nLow);

	return id;
}

int main(int argc, char* argv[])
{
	int ret;
	char *buffer;
	struct BlockId id;
	FILE *fp;
	/*for crc*/
	int crc, nlen;
	int crc_calc;
	int is_crc = 0;

	if(argc < 3) {
		printf("args error: ./ssd_read_id <block ID> <is_crc> [dump file] \n");
		return -1;
	}

	is_crc = atoi(argv[2]);
	if(4 == argc) {
		fp = fopen(argv[3], "ab");
		if(!fp) {
			printf("open file[%s] error\n", argv[3]);
			return -1;
		}
	}

	/*for crc*/
	if(is_crc) {
		key = mod2(CRC_DATA, CRC_POLY, CRC_POLY_BIT, CRC_DATA_BIT);
		gen_table(table, key);
	}

	id = argv2id(argv[1]);

	buffer = (char *)malloc(SIZE);
	if(!buffer) {
		printf("malloc error\n");
		ret = -1;
		goto CLOSE;
	}

	memset(buffer, 0, SIZE);

	/*	
	ret = SSD_open(0);
	if(ret < 0) {
		printf("open error: %d\n", ret);
		goto CLOSE_SSD;
	}
	*/	
	ret = SSD_read(MONITOR_DEV, id, buffer, SIZE, 0);	
	if(0 != ret)
		printf("read error: %d\n", ret);

	/*for crc*/
	if(is_crc) {
		crc = *(int *)buffer;
		nlen = *(int *)(buffer + 4);
		if(0 != crc) {
			crc_calc = baidu_crc32_fast(buffer + CRC_BLOCK_HEADER, 0, nlen - CRC_BLOCK_HEADER, table);
			if(crc_calc != crc) {
				printf("crc error: crc_orig[%d] VS crc_calc[%d]\n", crc, crc_calc);
			} else {
				printf("crc success: %d\n", crc);
			}
			
		}
	}

	if(4 == argc)	
		fwrite(buffer, SIZE, 1, fp);
	else
		fwrite(buffer, SIZE, 1, stdout);

CLOSE_SSD:
//	SSD_close_force(0);
	free(buffer);
CLOSE:
	if(4 == argc)
		fclose(fp);
	return ret;
}
