/*
 *  Copyright (C) 2012,   baidu_ssd
 *  <zhangwei29@baidu.com>, 2013.8
 *  read flush_tables from Reserved blocks in SSD
 */

#include "ssd_api.h"
#include <stdio.h>
#define NAME_LEN 30

//check parameters from users
int check_input(struct ssd_meta_version *version, int block, int page, ssd_u8 flag_block, int flag_page, int flag_fp)
{
    // check whether parameters  have been input 
    if ((1 != flag_page) || (1 != flag_block) || (1 != flag_fp)) {
        printf("parameters: -f<filename> -b<block> -p<page> or -v<version: not supported now!>\n");
        return -1;
    }
    //check whether parameters are valid 
    if ((block < 0) || (page < 0)) {
        printf(" block or page should not be negative!>\n");
        return -1;
    }

    //check whether parameters are valid 
    if ((block >= SSD_FLUSH_DATA_SIZE) || (page >= SSD_PAGES_PER_BLOCK)) {
        printf("Invalid input: block < %d & page < %d\n", SSD_FLUSH_DATA_SIZE, SSD_PAGES_PER_BLOCK);
        return -1;
    }

    version->block = (ssd_u16) block;
    version->page = (ssd_u8) page;
    return 0;
}


int main(int argc, char* argv[])
{
    int ret = -1, c;
    struct ssd_meta_version version;    
    char filename[NAME_LEN];
    FILE *fp = NULL;
    int block;
    int page;
    ssd_u8 flag_block = 0;
    ssd_u8 flag_page = 0;
    ssd_u8 flag_fp =0;

    while((c = getopt(argc, argv, "b:p:v:f:h")) > 0) {
        switch(c) {
            case 'h':
                printf("%s -f<filename> -b<block> -p<page> or -v<version: not supported now>\n", argv[0]);
                return ret;
            case 'b':
                block = atoi(optarg);
                flag_block = 1;
                break;
            case 'p':
                page = atoi(optarg);
                flag_page = 1;
                break;
            case 'v':
                printf("input version are not supported now\n");
                return ret;
            case 'f':
                strncpy(filename, optarg, NAME_LEN);
                flag_fp = 1;
                break;
            default:
                printf("error parameter input\n");
                printf("%s -f<filename> -b<block> -p<page> or -v<version: not support now>\n", argv[0]);
                return ret;
        }
    }
    if (check_input(&version, block, page, flag_block, flag_page, flag_fp) != 0)
        return ret;     

    //alloc buffer 
    version.buff = (char *)malloc(SSD_FLUSH_DATA_SIZE);
    if (!version.buff) {
        printf("malloc error\n");
        return ret;
    }
    
    ret = SSD_read_flush(&version);
    if (SSD_OK == ret) {
        fp = fopen(filename, "ab");
        if (fp) 
            fwrite(version.buff, SSD_FLUSH_DATA_SIZE, 1, fp);//write data into file
        else
            printf("open file[%s] error\n", optarg);
    } 

    if (fp != NULL)
        fclose(fp);
    free(version.buff);
    return ret;      
                
}
