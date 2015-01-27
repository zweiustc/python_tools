#include <pthread.h>
#include <stdlib.h>
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

//int check_error = 0;

typedef struct operate {
	int channel;
	int op;
	int blocks;
} operate_t;

int op_channel(operate_t *ope)
{
	int channel, op, blocks;
	struct BlockId id;
	char *buf = NULL, *rd_buf = NULL;
	int i, j, ret, cnt, err_cnt;
    int check_error = 0;
#ifdef CRC
	int crc_wr, crc_rd;
    FILE *fp = NULL;
    char filename[60];
#endif
 
    cnt = 0; 
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
			//buf[j] = random() % 26 + 'A';
			buf[j] = j % 256;
#ifdef CRC
		crc_wr = baidu_crc32_fast(buf, 0, BLOCKSIZE, table);
#endif
        cnt = 0;
		for(i = 0; i < DATABLOCKS; i++) {
            if(cnt >= blocks)
                break;
			id.m_nLow += SSD_MAX_CHANNEL;
			id.m_nHigh = (i << 1) | 1;

//			printf("ssd write %llx|%llx \n", id.m_nHigh, id.m_nLow);
			ret = SSD_write(0, id, buf, BLOCKSIZE, 0);
			if(ret == SSD_ID_ALREADY_EXIST) {
                continue;
            }
            else if(SSD_OK != ret) {
				printf("ssd write error %016lX%016lX %d\n", id.m_nHigh, id.m_nLow, ret);
				if(ret % 128)
                    check_error = 1;
				//goto out;
			}

            if(ret == SSD_OK) {
                cnt++;
                printf("ssd write ID: %016lX%016lX %d\n", id.m_nHigh, id.m_nLow, ret);
            }
			memset(rd_buf, 0, BLOCKSIZE);
			ret = SSD_read(0, id, rd_buf, BLOCKSIZE, 0);
			if(SSD_OK != ret) {
				printf("ssd read error %016lX%016lX %d\n", id.m_nHigh, id.m_nLow, ret);
				if(ret % 128)
                    check_error = 1;
				//goto out;
			}
 
            /*check the data with crc */
			crc_rd = baidu_crc32_fast(rd_buf, 0, BLOCKSIZE, table); 
            if(crc_wr != crc_rd) {
                printf("check crc error %08X vs %08X in %016lX%016lX\n", crc_wr, crc_rd, id.m_nHigh, id.m_nLow);
        
                for(j = 0; j < BLOCKSIZE; j++) {
                    if(buf[j] != rd_buf[j]) {
                        err_cnt++;
                        if(err_cnt < 5)
                            printf("check buf error %02X vs %02X in %016lX%016lX\n", (int) (unsigned char) buf[j], 
                                    (int) (unsigned char) rd_buf[j], id.m_nHigh, id.m_nLow);
                    }       
                }       
                printf("total error byte %d in %016lX%016lX\n", err_cnt, id.m_nHigh, id.m_nLow);

                sprintf(filename, "%08X", crc_wr);
                fp = fopen(filename, "ab");
                if(fp)  
                    fwrite(buf, BLOCKSIZE, 1, fp); 
                else    
                    printf("open file[%s] error\n", filename);
                fclose(fp);

        
        
                sprintf(filename, "%08X", crc_rd);
                fp = fopen(filename, "ab");
                if(fp)  
                    fwrite(rd_buf, BLOCKSIZE, 1, fp); 
                else    
                    printf("open file[%s] error\n", filename);
                fclose(fp);
   
                goto out;
            }	
		}
	}
out:
	if(rd_buf)
		free(rd_buf);
	if(buf)
		free(buf);
    
    if(check_error == 0) 
        return 0;
    else
        return -1; 
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

/* check the input parameters */
int check_input(int argc, char *argv[], int *block, int *channel)
{
    int test_blocks = 0, test_channel = -1, temp_channel = -1, i; 
	if(argc == 3) {
		test_blocks = atoi(argv[1]);
		if(test_blocks <= 0 || test_blocks >= DATABLOCKS) {
			printf("blocks[1~%u] error: %d\n", DATABLOCKS, test_blocks);
			return -1;
		}
        
        test_channel = atoi(argv[2]);
        if(test_channel < 0 || test_channel >= SSD_MAX_CHANNEL) {
            printf("channel[0~%u] error: %d\n", SSD_MAX_CHANNEL-1, test_channel);
            return -1;
        }
		printf("test channel: %d test blocks: %d\n", test_channel, test_blocks);
        
	}
 
    for(i = 0; i < 44; i++) {
        temp_channel = (i % 4) * 11 + (i / 4) % 11;
        if(temp_channel == test_channel) {
            test_channel = i;
            break;
        }
    }
    *block = test_blocks;
    *channel = test_channel;
    return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	operate_t ope;
	int test_blocks = 0;
    int test_channel = -1;

    ret = check_input(argc, argv, &test_blocks, &test_channel);
    if(ret != SSD_OK) {
        printf("./ssd_check_channel block_num channel_num\n");
        return -1;
    }

    /* open the device */
	ret = SSD_open(0);
	if(SSD_OK != ret) {
		printf("open error: %d\n", ret);
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
	ope.channel = test_channel;

	printf("step1: write + read %u blocks of channel %u\n", 
           (test_blocks == 0) ? DATABLOCKS : test_blocks, test_channel);
	ret = op_channel(&ope);
    if(ret != SSD_OK) {
        printf("check crc err !\n");
        //return -1;
    }
	SSD_close(0);

    return ret;
}
