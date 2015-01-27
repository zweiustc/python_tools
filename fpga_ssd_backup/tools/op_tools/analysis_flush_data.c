/*
 *  tools used for analysis of flush data
 *
 *  Copyright (C) 2001 baidu.com
 *
 *  2013-09-02  create by zhangwei<zhangwei29@baidu.com>
*/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "ssd_api.h"

#define FILE_LEN                        30
#define TABLE_UNIT                      2
#define TABLE_SIZE_PER_CHANNEL          SSD_PAGE_SIZE * TABLE_UNIT

//analysis addr tables
void analysis_addr(char *buffer, FILE *fp)
{
    int channel, unit;
    char *ptr;
    struct addr_entry *ptr_addr;
    fprintf(fp,"channel\t\tlogic_addr\t\tphy_addr\n");
    for (channel = 0;channel < SSD_MAX_CHANNEL;channel++) {
        ptr = buffer + channel * TABLE_SIZE_PER_CHANNEL * 2;
        ptr_addr = (struct addr_entry*) ptr;
        for (unit = 0;unit < SSD_MAX_PHYBLOCKS_PER_CHANNEL; unit++) {
            fprintf(fp,"channel = %d\t, logic_addr = %d\t, phy_addr = %d\t\n", channel, unit, ptr_addr->pb_addr);
            ptr_addr++;
        }
    }
}


//analysis erasetable
void analysis_erase(char *buffer, FILE *fp)
{

    int channel, unit;
    char *ptr;
    struct erase_entry *ptr_erase;
    fprintf(fp,"phy_addr\t\tbad_flag\t\tuse_flag\t\tera_count\t\n");
    for (channel = 0;channel < SSD_MAX_CHANNEL;channel++) {
        //jump to the first addr table location
        ptr = buffer +  TABLE_SIZE_PER_CHANNEL + channel * TABLE_SIZE_PER_CHANNEL * 2;
        ptr_erase = (struct erase_entry *) ptr;
        for (unit = 0;unit < SSD_MAX_PHYBLOCKS_PER_CHANNEL; unit++) {
            fprintf(fp,"channel = %d\t, phy_addr = %d\t, bad_flag = %d\t, use_flag = %d\t, era_count = %d \n",
            channel, unit, ptr_erase->bad_flag, ptr_erase->use_flag, ptr_erase->erase_times);
            ptr_erase++;
        }
    }
}

//analysi id table
void analysis_id(char *buffer, FILE *fp)
{
    int unit;
    struct flush_id_info *ptr;
    fprintf(fp,"ID\t\tchannel\t\tlen\t\tcrc32");
    ptr = (struct flush_id_info *) (buffer + SSD_MAX_CHANNEL * TABLE_SIZE_PER_CHANNEL * 2);
    for (unit = 0; unit < SSD_MAX_VIRBLOCKS_PER_CHANNEL * SSD_MAX_CHANNEL; unit++) {
        if (ptr->id.m_nLow || ptr->id.m_nHigh) {
            fprintf(fp,"ID = %x%x\t, channel = %d\t, block = %d\t, len = %d\t, crc = %x\n", ptr->id.m_nLow, 
            ptr->id.m_nHigh, ptr->channel, ptr->len, ptr->crc32);
            ptr++;
        }
    }
}
//check the input is suitable and complete
int check_file(int source_flag, char *source_file, FILE *fp)
{
    int ret = -1, file_len;
    // check whether the filename has been input
    if (0 == source_flag) {
        printf(" -f <souce flush_data filename>  should be added!\n");
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
    if (file_len != SSD_FLUSH_DATA_SIZE) {
        printf("FLUSH DATA file is not complete!\n");
        return ret;
    }
    fseek(fp, 0L, SEEK_SET);
    return 0;
}

int main(int argc, char *argv[])
{
    int ret = -1;
    int c;
    int source_flag = 0;
    char source_file[FILE_LEN];
    FILE *fp = NULL;
    char *buffer;
    
    // get source file from user 
    while ((c = getopt(argc, argv, "f:h")) > 0) {
        switch(c) {
            case 'h':
                printf("%s -f <souce flush_data filename> \n", argv[0]);
                return ret;
            case 'f':
                source_flag = 1;
                strncpy(source_file, optarg, FILE_LEN);
                break;
            default:
                printf("%s -f <souce flush_data filename> \n", argv[0]);
                return ret;
        }
    }


    fp = fopen(source_file, "rb");
    if ((check_file(source_flag, source_file, fp)) != 0)
        return ret;

    //malloc buffer
    buffer = (char *)malloc(SSD_FLUSH_DATA_SIZE);
    if (buffer == NULL) {
        printf("malloc error!\n");
        return ret;
    }
    memset(buffer, 0, SSD_FLUSH_DATA_SIZE); 

    //read file content
    fread(buffer, SSD_FLUSH_DATA_SIZE, 1, fp);
   
    //create and open the file for the result
    fp = fopen("flush_result", "wb");
    if (fp == NULL) {
        printf("Failed to open file: %s\n", source_file);
        return ret;
    }
     
    analysis_addr(buffer, fp);
    analysis_erase(buffer, fp);
    analysis_id(buffer, fp);

    fclose(fp);
    free(buffer);
    return 0;
}

