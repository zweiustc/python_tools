//#include "ssd_api.c"
#include "cfg_online.h"
#include "cfg_online.c"

int main(int argc, char *argv[])
{
    uint64_t addr;
    int version;
    int ret;
    
    if (argc < 2)
    {
        ret = -CFG_PARAM_ERROR;
        printf("Please input S6 verison number!\n");
    }
    else
    {
        sscanf(argv[1], "%d", &version);
        
        if (version >= 1 && version <= 3)
        {
            if (version == 1)
                addr = S6_ADDR1;
            else if (version == 2)
                addr = S6_ADDR2;
            else
                addr = S6_ADDR3;
                
            printf("Configure S6 in verison %d :\n",version);
            
            /*ret = SSD_open_newcard(0);
            if (ret != SSD_OK) {
                printf("Open error = %d\n", ret);
                return EXIT_SUCCESS;
            }*/
        
            ret = cfg_s6(addr);
            //sleep(2);
            //SSD_close(0);
        }
        else
        {
            printf("The verison number should only be 1,2,3!\n");
            ret = -CFG_PARAM_ERROR;
        }
    }
    return ret;
}


