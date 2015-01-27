/*
 *  clear_all .c
 *
 *   define  for rhe function ssd_open test
 *
 *  Copyright (C) 2001 baidu.com
 *
 *  2012-11-11  create by liyang<liyang_15@baidu.com>
*/



#include <stdio.h>
#include <fcntl.h>
#include "../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../include/test_tools.h"
#include "../include/test_macro.h"

int main(void)
{
	//enum SSD_ERROR_TYPE ssd_flag = SSD_ERROR;
    	int ssd_flag =SSD_ERROR;
	enum SSD_TEST_RET_TYPE ret;
    	ret=SSD_TEST_OK;
    
	printf("case clear_all: SSD_open()\n");
/*
   	ssd_flag=SSD_open(0);
	if (ssd_flag!= SSD_OK) {
		printf("\tERROR!\n");
		printf("\tcase clear_all, excepted: %d, actual: %d\n", SSD_OK, ssd_flag);
		ret = SSD_OPEN_ERROR;
        	exit(-1);
    	} 
    	else {
		printf("case clear_all: SSD_OPEN returns successfully\n");
	}
    
  */ 
   	printf("case open newcard:open the new ssdcard");
   	ssd_flag=SSD_open_newcard(0);
	if (ssd_flag!= SSD_OK) {
		printf("\tERROR!\n");
		printf("\tcase open newcard, excepted: %d, actual: %d\n", SSD_OK, ssd_flag);
		ret = SSD_FTW_ERROR;
        exit(-1);
    	} 
    	else {
		printf("case open newcard: SSD_open_newcard returns successfully\n");

    	}
    
    	printf("case clear_all: SSD_close \n");
    	SSD_close(0);
	
	if (SSD_TEST_OK == ret)
		printf("test cases open newcard:   PASS!!!\n");
	else
		printf("test cases open newcard:   FAIL!!!\n");

    
    return 0;
    //the problem;
}

