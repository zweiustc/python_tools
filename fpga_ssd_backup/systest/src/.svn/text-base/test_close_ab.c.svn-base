#include <stdio.h>
#include <time.h>
#include "../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../include/test_tools.h"
#include "../include/test_macro.h"

int main(void)
{
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	int ssd_flag=SSD_ERROR;
        /*	
	printf("case 02-02-01: SSD_close() but not ssd_open before\n");    
	if ((ssd_flag = SSD_close()) != -SSD_ERR_NOT_OPENED) {
		printf("\tERROR!");
		printf("\tcase 02-02-01, expected: %d, actual: %d\n", -SSD_ERR_NOT_OPENED, ssd_flag);
		ret = SSD_CLOSE_ERROR;
        exit(-1);
	} else {
		printf("SSD_ERR_NOT_SSD returns successfully\n");
	}

 	printf("case 02-02-02: ssd_open()\n");
	if ((ssd_flag = SSD_open()) != SSD_OK) {
		printf("\tERROR!\n");
		printf("\tcase02-02-02, excepted: %d, actual: %d\n", SSD_OK, ssd_flag);
		ret = SSD_OPEN_ERROR;
        exit(-1);
	} else {
		printf("case 02-02-02: SSD_OPEN returns successfully\n");
	}

	printf("case 02-02-03: SSD_close\n");	
	if ((ssd_flag = SSD_close()) != SSD_OK) {
		printf("\tERROR!\n");
		printf("\tcase 02-02-03, excepted: %d, actual: %d\n", SSD_OK, ssd_flag);
		ret = SSD_CLOSE_ERROR;
        exit(-1);
	} else {
		printf("case 02-02-03: SSD_CLOSE returns successfully\n");
	}
	
	printf("case 02-02-04: SSD_close again \n");	
	if ((ssd_flag = SSD_close()) != -SSD_ERR_NOT_OPENED) {
		printf("\tERROR");
		printf("\tcase 02-02-04, excepted: %d, actual: %d\n", SSD_ERR_NOT_OPENED, ssd_flag);
		ret = SSD_CLOSE_ERROR;
        exit(-1);
	} else {
		printf("case 02-02-04: SSD_CLOSE abnormal returns successfully\n");
	}

        */
	
	SSD_close_force(0);
	
	if (ret == SSD_TEST_OK)
	SSD_CASE_INFO("02-02","ALL","close SSD abnormal test FAIL");
	else
        SSD_CASE_INFO("02-02","ALL","close SSD abnormal test PASS");
	return 0;
}

