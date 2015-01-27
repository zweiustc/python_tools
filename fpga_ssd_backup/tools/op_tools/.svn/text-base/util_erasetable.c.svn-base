#include "ssd_api.h"
#include <stdio.h>
#include <unistd.h>


void print_total(void)
{
    uint64_t free_all = 0;
    int i;
    free_all = 0;
    for (i = 0; i < SSD_MAX_CHANNEL; i++)
    {
        free_all += SSD_util_test(0, i);
    }
    printf("Total:     \t%llu\t\t\t%u\n",
       free_all, free_all / SSD_BLOCK_SIZE);
}

void print_channel(int channel)
{
    uint64_t free_space = 0;
    free_space =SSD_util_test(0, channel);
    printf("Channel[%u]\t%llu\t\t\t%u\n",
           channel, free_space, free_space / SSD_BLOCK_SIZE);
}

void print_head(void)
{
    printf("----------------------   Free   Space   ----------------------\n");
    printf("Channel       \tFree Bytes\t\t\tFree Blocks\n");
}

int main(int argc, char *argv[])
{
    int i;
    int ret;
    int mode;
    int ch;
    int sleep_time = 2;
    int total_mode = 0;
    int channel_mode = 0;
    int total_and_channel_mode = 1;
    int channel;
    
    
        
    //parameter check  
    opterr = 0;
    while((ch = getopt(argc,argv,"c:i:s"))!= -1)
        switch(ch)
        {
            case 's':
                total_mode = 1;
                total_and_channel_mode = 0;
                break;
            case 'i':
                sleep_time = atoi(optarg);
                if (sleep_time <= 1)
                    sleep_time = 1;
                break; 
            case 'c':
                channel = atoi(optarg);
                if (channel < 0 || channel > 43) {
                    printf("Illegal channel number!\n");
                    return EXIT_SUCCESS;
                }
                
                channel_mode = 1;
                total_and_channel_mode = 0;
                break; 
            default:
                printf("Illegal parameter!\n");
                return EXIT_SUCCESS;
        }
        
    while(1)
    {
        print_head();
        
        if (total_mode || total_and_channel_mode)
            print_total();
            
        if (channel_mode)
            print_channel(channel);
        else if(total_and_channel_mode)//fault print
            for (i = 0; i < SSD_MAX_CHANNEL; i++)
                print_channel(i);
        
        if(total_and_channel_mode)
            printf("\n\n\n");
        
        sleep(sleep_time);
    }
        
    return EXIT_SUCCESS;
}


