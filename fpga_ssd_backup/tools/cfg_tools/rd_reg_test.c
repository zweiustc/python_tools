//#include "ssd_api.c"
#include "ssd_api.h"

int main(int argc, char *argv[])
{
    int ret;
    uint64_t ans;
    uint64_t value;
    if (argc > 1)
    {
        /*ret = SSD_open(0);
        if (ret != SSD_OK) {
            printf("Open error = %d\n", ret);
            return EXIT_SUCCESS;
        }*/
            
        sscanf(argv[1], "%llx", &value);
        ans = ssd_reg_read(value);
        printf("receive data:%llx\n", ans);
        
        //SSD_close(0);
    }
    
    return EXIT_SUCCESS;
}


