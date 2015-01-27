#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include "ssd_api.h"


int spi_write_ram(unsigned char* buf, int write_count);
unsigned char hex2int(char * hex);
int file_change(const char* mcs_pathname, const char* bin_pathname);

int spi_write_ram(unsigned char* buf, int write_count)
{
    uint64_t i;
    uint64_t index = 0;
    uint64_t write_data;
    int reg_ans;
    int error_num = 0;
    for(i = 0; i < write_count; i += 8)
    {
       write_data = (((uint64_t)buf[index+7]) << 56 ) | 
                    (((uint64_t)buf[index+6]) << 48 ) | 
                    (((uint64_t)buf[index+5]) << 40 ) |
                    (((uint64_t)buf[index+4]) << 32 ) |
                    (((uint64_t)buf[index+3]) << 24 ) |
                    (((uint64_t)buf[index+2]) << 16 ) |
                    (((uint64_t)buf[index+1]) << 8 ) | 
                    (((uint64_t)buf[index]) );

        reg_ans = ssd_reg_write(SPI_RAM_START +i, write_data);
        if(reg_ans != 0)
            error_num++;
        index += 8;
    }
    if(error_num)
        printf("Error type: %d\n",reg_ans);//the last error type
    
    return error_num;
}

int cfg_flash(const char* bin_pathname)
{
    FILE* fd;
    uint64_t read_count;
    unsigned char buf[2 * MAXLEN];
    int timeout;
    uint64_t start;
    int error_num;
    uint64_t status;

    fd = fopen(bin_pathname, "rb");
    if (fd == NULL)  
    {
        printf("No such file!\n");
        return -CFG_NO_FILE;
    }
    
    
    //read status
    timeout = 0;
    status = ssd_reg_read(SPI_ADDR_STATUS);
    if(status & 0x0FC)
        printf("status=0x%llx\n", status);
    
    while( (ssd_reg_read(SPI_ADDR_STATUS) & 0x01) || 
            (ssd_reg_read(0x8068) & 0x0f0000) )
    {
        sleep(1);
        timeout++;
        if(timeout > SPI_TIME_READ_STATUS)
        {
            fclose(fd);
            printf("Read status timeout!\n");
            return -CFG_READ_STATUS_TIMEOUT;
        }
        status = ssd_reg_read(SPI_ADDR_STATUS);
        if(status & 0x0FC) printf("status=0x%llx\n",status);
    }
    
    
    printf("Start erase flash......\n\n");
    //send erase command
    ssd_reg_write(SPI_ADDR_CTRL, CMD_ERASE);
    usleep(1000);   
 
    //read status
    timeout = 0;
    status = ssd_reg_read(SPI_ADDR_STATUS);
    if(status & 0x0FC)
        printf("status=0x%llx\n", status);
    
    while( (ssd_reg_read(SPI_ADDR_STATUS) & 0x01) || 
            (ssd_reg_read(0x8068) & 0x0f0000) )
    {
        sleep(1);
        timeout++;
        if(timeout > SPI_TIME_ERASE)
        {
            fclose(fd);
            printf("Erase status timeout!\n");
            return -CFG_ERASE_TIMEOUT;
        }
        
        status = ssd_reg_read(SPI_ADDR_STATUS);
        if(status & 0x0FC)
            printf("status=0x%llx\n",status);
   
    }
    printf("Erase flash sucess!\n\n");

    printf("Start sending flash data.........\n");
    //send config data
    start = 0;
    while((read_count = fread(buf, sizeof(unsigned char), MAXLEN, fd)) != 0)
    {
        //send data to ram
        error_num = spi_write_ram(buf, read_count);
        if( error_num !=0 )
            printf("Write flash: %d error at addr = %llx \n",error_num,start);
   
        //send write command
        ssd_reg_write(SPI_ADDR_CTRL,start | CMD_WRITE);
        usleep(1000);   
        
        //print message
        if(start % 0x10000 ==0)
            printf("Write flash at addr = 0x%llx\n",start);
        start += 256;
        
        //read status
        status = ssd_reg_read(SPI_ADDR_STATUS);
        if(status & 0x0FC)
            printf("status=0x%llx\n",status);
        
        while( (ssd_reg_read(SPI_ADDR_STATUS) & 0x01) || 
            (ssd_reg_read(0x8068) & 0x0f0000) )
        {
            status = ssd_reg_read(SPI_ADDR_STATUS);
            if(status & 0x0FC)
                printf("status=0x%llx\n",status);
        } 
    }
          
    fclose(fd);
    printf("Write flash success!\n");
    return CFG_OK;    
}

int cfg_s6(uint64_t addr)
{
    int timeout;
    
    //read status
    timeout = 0;
    while ( (ssd_reg_read(SPI_ADDR_STATUS) & 0x01) || 
            (ssd_reg_read(0x8068) & 0x0f0000) ){
        sleep(1);
        timeout++;
        if (timeout > SPI_TIME_READ_STATUS){
            printf("Read status timeout!\n");
            return -CFG_READ_STATUS_TIMEOUT;
        }
    }
    
    //send read command
    ssd_reg_write(SPI_ADDR_CTRL, addr | CMD_READ);
    usleep(1000);   
    
    //read status
    sleep(1);
    timeout = 0;
    while ( (ssd_reg_read(SPI_ADDR_STATUS) & 0x01) ||
            (ssd_reg_read(0x8068) & 0x0f0000) ){
        sleep(1);
        timeout++;
        if (timeout > SPI_TIME_READ_STATUS){
            printf("Read status timeout!\n");
            return -CFG_READ_STATUS_TIMEOUT;
        }
    }  
    printf("Config S6 sucess!\n");
    return CFG_OK;    
}

//file format change
unsigned char hex2int(char * hex)
{
    unsigned char data = 0;
    if (*hex >= '0' && *hex <= '9')
        data += (*hex - '0');
    else if (*hex >= 'A' && *hex <= 'F')
        data += (*hex - 'A' + 10);
        
    data *= 16;
     
    if (*(hex+1) >= '0' && *(hex+1) <= '9')
        data += ( *(hex+1) - '0' );
    else if (*(hex+1) >= 'A' && *(hex+1) <= 'F')
        data += (*(hex+1) - 'A' + 10);    

    return data;
}

int file_change(const char* mcs_pathname, const char* bin_pathname)
{   

    FILE *fd1,*fd2;
    //MCS define
    char one_line[BUF_SIZE];
    unsigned long length;
    //address setting
    unsigned long high_addr,low_addr,curr_addr;
    //temp for checksum test
    unsigned char chk_data;
    unsigned int i;
    int k;
    unsigned char tmp_char;
    unsigned char empty = EMPTY_BYTE;
    unsigned int line_num;

    //set & open files
    fd1 = fopen(mcs_pathname, "rc");
    if (fd1 == NULL)  
    {
        printf("File foramt change: No such file: %s !\n", mcs_pathname);
        return 0;
    }
        
    fd2 = fopen(bin_pathname, "wb");
    if (fd2 == NULL)  
    {
        printf("File foramt change: Creat temp file error!\n");
        return EXIT_SUCCESS;
    }


    line_num = 0;
    high_addr = 0;
    low_addr = 0;
    curr_addr = 0;
    //start reading
    while (fgets(one_line, BUF_SIZE, fd1) != NULL)
    {
        length = hex2int(one_line + 1);
        
        //step1: checksum test
        chk_data = 0;
        for (i = 1; i < length * 2 + 11; i += 2)
            chk_data += hex2int(one_line+i);
        
        if(chk_data != 0)
        {
            printf("File foramt change: Checksum error: line %d",line_num + 1);
            fclose(fd1);
            fclose(fd2);
            return EXIT_SUCCESS;
        }
        
        //step2: slove record
        //Extend linear address record
        if(one_line[7]=='0' && one_line[8]=='4' )
        {
            //change high_addr
            high_addr = (hex2int(one_line + 9) << 8) + hex2int(one_line + 11);
        }
        
        //Data record
        else if(one_line[7]=='0' && one_line[8]=='0' )
        {
            //change low_addr
            low_addr = (hex2int(one_line+3) << 8) + hex2int(one_line + 5);
            //add empty data
            while (curr_addr < (high_addr << 16) + low_addr)
            {
                fwrite( &empty, sizeof(unsigned char), 1, fd2 );
                curr_addr++;
            }
            
            //write current data
            k = 9;//start tip of data in data record
            while (curr_addr < (high_addr <<16) + low_addr + length)
            {
                
                tmp_char = hex2int( one_line+k );
                fwrite( &tmp_char, sizeof(unsigned char), 1, fd2 );
                k+=2;
                curr_addr++;
            }
        }
        
        //End-of-file record
        else if(one_line[7]=='0' && one_line[8]=='1' )
        {
            break;
        }
        
        //unsupported record
        else
        {
            printf("File foramt change: Unsupported record!\n");
            fclose(fd1);
            fclose(fd2);
            return EXIT_SUCCESS;
        }

        line_num++;
    }
    
    //step3: add empty data in the end
    while (curr_addr < MAX_FLASH_ADDR)
    {
        fwrite(&empty, sizeof(unsigned char), 1, fd2);
        curr_addr++;
    }
    
    fclose(fd1);
    fclose(fd2);
    printf("File foramt change sucess!\n");
    return EXIT_SUCCESS;
}





