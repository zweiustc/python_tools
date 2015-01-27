/*
 *  tools used for get crc32 of id data
 *
 *  Copyright (C) 2001 baidu.com
 *
 *  2013-09-25  create by zhangwei<zhangwei29@baidu.com>
*/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "ssd_api.h"

unsigned long long table[256];
unsigned long long key;
#define CRC_DATA 0x1LL
#define CRC_POLY 0x11edc6f41LL
#define CRC_DATA_BIT (1024 * 8)
#define CRC_POLY_BIT 33

#define FILE_LEN                        100
#define TABLE_UNIT                      2
#define TABLE_SIZE_PER_CHANNEL          SSD_PAGE_SIZE * TABLE_UNIT
#define BLOCKSIZE                       (5 * 2 * 1024 * 1024)
//check the input is suitable and complete
int check_file(int source_flag, char *source_file, FILE *fp)
{
    int ret = -1, file_len;
    // check whether the filename has been input
    if (0 == source_flag) {
        printf(" -f <souce crc32 filename>  should be added!\n");
        return ret;
    } 

    //source file open failed
    if (fp == NULL) {
        printf("Failed to open file: %s\n", source_file);
        return ret;
    }
    
    //check file length
    fseek(fp, 0L, SEEK_END);
    file_len = ftell(fp);
    if (file_len != BLOCKSIZE) {
        printf("FLUSH DATA file is not complete!\n");
        return ret;
    }
    fseek(fp, 0L, SEEK_SET);
    return 0;
}

int main(int argc, char *argv[])
{
    int ret = -1;
    int c, crc_rd;
    int source_flag = 0;
    char source_file[FILE_LEN];
    FILE *fp = NULL;
    char *buffer;
    
    // get source file from user 
    while ((c = getopt(argc, argv, "f:h")) > 0) {
        switch(c) {
          case 'h':
                printf("%s -f <souce crc32 filename> \n", argv[0]);
                return ret;
            case 'f':
                source_flag = 1;
                strncpy(source_file, optarg, FILE_LEN);
                break;
            default:
                printf("%s -f <souce crc32 filename> \n", argv[0]);
                return ret;
        }
    }


    fp = fopen(source_file, "rb");
    if ((check_file(source_flag, source_file, fp)) != 0)
        return ret;

    //malloc buffer
    buffer = (char *)malloc(BLOCKSIZE);
    if (buffer == NULL) {
        printf("malloc error!\n");
        return ret;
    }
    memset(buffer, 0, BLOCKSIZE); 

    //read file content
    fread(buffer, BLOCKSIZE, 1, fp);
   
    //create and open the file for the result
    fp = fopen("flush_result", "wb");
    if (fp == NULL) {
        printf("Failed to open file: %s\n", source_file);
        return ret;
    }
     
	/*init crc*/
	key = mod2(CRC_DATA, CRC_POLY, CRC_POLY_BIT, CRC_DATA_BIT);
	gen_table(table, key);

	crc_rd = baidu_crc32_fast(buffer, 0, BLOCKSIZE, table);
    printf("the crc32 result is : %X\n", crc_rd);

    fclose(fp);
    free(buffer);
    return 0;
}


