//#include "ssd_api.c"
#include "cfg_online.h"
#include "cfg_online.c"

int main(int argc, char *argv[])
{
    
    ssd_reg_write(0x80f0, 0);
    usleep(1000);   

    return 0;
}


