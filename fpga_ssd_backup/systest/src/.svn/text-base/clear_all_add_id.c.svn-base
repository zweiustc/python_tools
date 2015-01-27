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
    	uint64_t util_flag;

	SSD_CASE_INFO("","ALL","delete the add nformation in SSD ");
    
	SSD_CASE_INFO("","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("","01","OPEN THE SSD DEVICE");
        }

	SSD_CASE_INFO("","02","check the util info");
   	util_flag = SSD_util(0);
	SSD_RESULT("uril"," ", " ",util_flag,0,0 );
   
    
//   	gb_filecount=0;
	SSD_CASE_INFO("","03","delete the ftw info");
   	ssd_flag= SSD_ftw(0,delete_all_add_id, 0);
	if (ssd_flag!= SSD_OK) {
		ret = SSD_FTW_ERROR;
		SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        	SSD_ERR("","03","do the ftw");
		
    	} 
    	else {
		SSD_SUCCESS("","03"," do the ftw");

   	}

   
	SSD_close(0);
	
	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("","ALL","delete the add information in SSD test pass");
	else
		SSD_CASE_INFO("","ALL","delete the add information in SSD test pass ");

    
    return 0;
    //the problem;
}

