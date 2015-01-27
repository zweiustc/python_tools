#include <stdio.h>
#include "ssd_api.h"


int main()
{
	int ret;
    
	printf("case clear_all: SSD_open()\n");

   	printf("case open newcard:open the new ssdcard");
   	ret = SSD_open_newcard(0);
	if (ret != SSD_OK) {
		printf("\tERROR!\n");
		printf("\tcase open newcard, excepted: %d, actual: %d\n", SSD_OK, ret);
    	} 
    	else {
		printf("case open newcard: SSD_open_newcard returns successfully\n");
    	}
    //
    	printf("case clear_all: SSD_close \n");
    	//SSD_close(0);
	
	if (SSD_OK == ret)
		printf("test cases open newcard:   PASS!!!\n");
	else
		printf("test cases open newcard:   FAIL!!!\n");

    
    return 0;
}

