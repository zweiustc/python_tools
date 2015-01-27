#include <pthread.h>
#include "ssd_api.h"

#define RESERVED (1000 / 5)
#define BLOCKSIZE (5 * 2 * 1024 * 1024)
#define DATABLOCKS ((SSD_MAX_VIRBLOCKS_PER_CHANNEL / 5) - RESERVED)
#define SECBLOCKS ((200 / 5) + RESERVED)

#define WRRD 0
#define ERASE 1

/*for crc*/
#define CRC
#ifdef CRC
unsigned long long table[256];
unsigned long long key;

#define CRC_DATA 0x1LL
#define CRC_POLY 0x11edc6f41LL
#define CRC_DATA_BIT (1024 * 8)
#define CRC_POLY_BIT 33
#endif

int check_error = 0;

typedef struct operate {
	int channel;
	int op;
	int blocks;
} operate_t;

void *worker(void *arg)
{
	operate_t *ope = (operate_t *)arg;
	int channel, op, blocks;
	struct BlockId id;
	char *buf = NULL, *rd_buf = NULL;
	int i, j, ret;
#ifdef CRC
	int crc_wr, crc_rd;
#endif

	channel = ope->channel;
	op = ope->op;
	blocks = ope->blocks;
//	printf("channel: %d, blocks: %d\n", channel, blocks);

	id.m_nLow = channel;

	if(ERASE == op) {
		for(i = 0; i < blocks; i++) {
			id.m_nLow += SSD_MAX_CHANNEL;
			id.m_nHigh = (i << 1) | 1;
			ret = SSD_delete(0, id);
			if(SSD_OK != ret) {
				printf("delete id[%016lX%016lX] error: %d\n", id.m_nHigh, id.m_nLow, ret);
			}
		}
	} else {
		buf = (char *)malloc(BLOCKSIZE);
		if(!buf) {
			printf("alloc buf error\n");
			return;
		}

		rd_buf = (char *)malloc(BLOCKSIZE);
		if(!rd_buf) {
			printf("alloc rd_buf error\n");
			free(buf);
			return;
		}

		memset(buf, 0, BLOCKSIZE);
		for(j = 0; j < BLOCKSIZE; j++)
			buf[j] = random() % 26 + 'A';
#ifdef CRC
		crc_wr = baidu_crc32_fast(buf, 0, BLOCKSIZE, table);
#endif

		for(i = 0; i < blocks; i++) {
			id.m_nLow += SSD_MAX_CHANNEL;
			id.m_nHigh = (i << 1) | 1;

//			printf("ssd write %llx|%llx \n", id.m_nHigh, id.m_nLow);
			ret = SSD_write(0, id, buf, BLOCKSIZE, 0);
			if(SSD_OK != ret) {
				printf("ssd write error %016lX%016lX %d\n", id.m_nHigh, id.m_nLow, ret);
				check_error = 1;
//				goto out;
			
			}

			memset(rd_buf, 0, BLOCKSIZE);
			ret = SSD_read(0, id, rd_buf, BLOCKSIZE, 0);
			if(SSD_OK != ret) {
				printf("ssd read error %016lX%016lX %d\n", id.m_nHigh, id.m_nLow, ret);
				check_error = 1;
//				goto out;
			}

			/*check data*/
#ifdef CRC
			crc_rd = baidu_crc32_fast(rd_buf, 0, BLOCKSIZE, table);
			if(crc_wr != crc_rd) {
				check_error = 1;
				printf("check error %d vs %d in %016lX%016lX\n", crc_wr, crc_rd, id.m_nHigh, id.m_nLow);
//				goto out;
			}
#else
			for(j = 0; j < BLOCKSIZE; j++) {
				if(buf[j] != rd_buf[j]) {
					check_error = 1;
					printf("check error %c vs %c in %016lX%016lX\n", buf[j], rd_buf[j], id.m_nHigh, id.m_nLow);
//					goto out;
				}
			}
#endif
		}
	}

out:
	if(rd_buf)
		free(rd_buf);
	if(buf)
		free(buf);
}

void operate_blocks(operate_t ope)
{
	int channel;
	operate_t *op[SSD_MAX_CHANNEL];
	pthread_t tid[44];

	for(channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
		op[channel] = (operate_t *)malloc(sizeof(operate_t));
		if(!op[channel]) {
			printf("malloc operation error\n");
			goto out;
		}

		op[channel]->op = ope.op;
		op[channel]->blocks = ope.blocks;
		op[channel]->channel = channel;
		pthread_create(&tid[channel], NULL, worker, (void *)op[channel]);
	}
out:	
	for(channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
		pthread_join(tid[channel], NULL);
		if(op[channel])
			free(op[channel]);
	}
}

struct BlockId char2id(const char *str)
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

int delete_all_file(const char *fid, const struct stat *st, int flag)
{
	int ret = -1;
	ret = SSD_delete(0, char2id(fid));
//	printf("delete %s: %d\n", fid, ret);
	return ret;
}

int main(int argc, char *argv[])
{
	int ret;
	operate_t ope;
	int test_blocks = 0;

	if(argc == 2) {
		test_blocks = atoi(argv[1]);
		if(test_blocks <= 0 || test_blocks >= DATABLOCKS) {
			printf("blocks[1~%u] error: %d\n", DATABLOCKS, test_blocks);
			return -1;
		}
		printf("test blocks: %d\n", test_blocks);
	}

	ret = SSD_open(0);
	if(SSD_OK != ret) {
		printf("open error: %d\n", ret);
		return -1;
	}
	/*erase all exists ID*/
	ret = SSD_ftw(0, delete_all_file, 0);
	if(SSD_OK != ret) {
		printf("erase id error %d\n", ret);
		SSD_close(0);
		return -1;
	}
#ifdef CRC
	/*init crc*/
	key = mod2(CRC_DATA, CRC_POLY, CRC_POLY_BIT, CRC_DATA_BIT);
	gen_table(table, key);
#endif

	/*write + read 44 * 8000 blocks*/
	ope.op = WRRD;
	ope.blocks = (test_blocks == 0) ? DATABLOCKS : test_blocks;
	ope.channel = -1;

	printf("step1: write + read 44 * %u blocks\n", (test_blocks == 0) ? DATABLOCKS : test_blocks);
	operate_blocks(ope);

	SSD_close(0);

	/*erase 44 * 8000 blocks*/
	if(check_error)
		return -1;

	ret = SSD_open(0);
	if(SSD_OK != ret) {
		printf("open error: %d\n", ret);
		return -1;
	}

	ret = SSD_ftw(0, delete_all_file, 0);
	if(SSD_OK != ret) {
		printf("erase id error %d\n", ret);
		SSD_close(0);
		return -1;
	}
	SSD_close(0);

	/*write + read 44 * 200 blocks*/
	if(test_blocks)
		return 0;

	ret = SSD_open(0);
	if(SSD_OK != ret) {
		printf("open error: %d\n", ret);
		return -1;
	}

	ope.op = WRRD;
	ope.blocks = SECBLOCKS;
	ope.channel = -1;

	printf("step2: write + read 44 * %u blocks\n", SECBLOCKS);
	operate_blocks(ope);

	SSD_close(0);
	/*erase 44 * 8000 blocks*/
	if(check_error)
		return -1;

	ret = SSD_open(0);
	if(SSD_OK != ret) {
		printf("open error: %d\n", ret);
		return -1;
	}

	ret = SSD_ftw(0, delete_all_file, 0);
	if(SSD_OK != ret) {
		printf("erase id error %d\n", ret);
		SSD_close(0);
		return -1;
	}

	SSD_close(0);
	return 0;
}
