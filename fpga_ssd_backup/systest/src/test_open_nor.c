/*
 *  test_open .c
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
#include "../include/test_tools.h"
#include "../include/test_macro.h"

int main(void)
{
	//enum SSD_ERROR_TYPE ssd_flag = SSD_ERROR;
    	int ssd_flag =SSD_ERROR;
		enum SSD_TEST_RET_TYPE ret;
    	ret=SSD_TEST_OK;

	
	SSD_CASE_INFO("01-01","ALL","TEST open normal ");
    
	SSD_CASE_INFO("01-01","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("01-01","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("01-01","01","OPEN THE SSD DEVICE");
        }  
 
    
	SSD_CASE_INFO("01-01","02","close THE SSD DEVICE");  
	SSD_close(0);
	
	if (SSD_TEST_OK == ret)
	
		SSD_CASE_INFO("01-01","ALL","open SSD normal pass");
	else
		SSD_CASE_INFO("01-01","ALL","open SSD normal fail");

    
    return 0;
  
}

