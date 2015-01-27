#include <stdio.h>
#include "ssd_api.h"

int main()
{
    int ret;
    
    ret = SSD_flush(0);
    
    if (ret != SSD_OK)
        printf("FLUSH ERROR! ret is %d\n", ret);
    
    return ret;
}

