//#include "ssd_api.c"
#include "cfg_online.h"
#include "cfg_online.c"

int main(int argc, char *argv[])
{
    FILE *fd;
    int ret;
    
    if (argc > 1)
    {
        /*ret = SSD_open_newcard(0);
        if (ret != SSD_OK) {
            printf("Open error = %d\n", ret);
            return EXIT_SUCCESS;
        }*/
    
        file_change(argv[1], "file_tmp.tmp");
        ret = cfg_flash("file_tmp.tmp");
        if (ret != -CFG_NO_FILE)
            remove("file_tmp.tmp");
        if (ret == CFG_OK)
            printf("Configure flash sucess!\n");
        //sleep(2);
        //SSD_close(0);
    }
    else
    {
        ret = -CFG_PARAM_ERROR;
        printf("please input file name!\n");
    }
    return ret;
}


