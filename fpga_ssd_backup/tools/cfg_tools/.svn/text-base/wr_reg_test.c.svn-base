//#include "ssd_api.c"
#include "ssd_api.h"

int main(int argc, char *argv[])
{
    int ret;
    uint64_t value1,value2;
    
    if (argc > 2)
    {
        /*ret = SSD_open(0);
        if (ret != SSD_OK) {
            printf("Open error = %d\n", ret);
            return EXIT_SUCCESS;
        }*/
            
        sscanf(argv[1], "%llx", &value1);
        sscanf(argv[2], "%llx", &value2);
        printf("addr: %llx   data: %llx\n", value1, value2);
        ret = ssd_reg_write(value1, value2);
        //sleep(2);
        //SSD_close(0);
    }
    return EXIT_SUCCESS;
}


