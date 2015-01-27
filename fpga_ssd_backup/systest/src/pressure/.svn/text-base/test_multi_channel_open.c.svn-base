#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include "../../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../../include/test_tools.h"
#include "../../include/test_macro.h"
 

 


int main(void)
{
	int i,ssd_flag;
	int NUM,SSD_THREAD=100;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[100];
	void *tret;
    	
	BlockId id;
 
  
 
	SSD_CASE_INFO("a-operation","ALL","TEST write the whole SSD 90%");


        SSD_CASE_INFO("a-o","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("a-o","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("a-o","01","OPEN THE SSD DEVICE");
        }
 
	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("a-o","ALL","TEST compare util flush pass");
	else
		SSD_CASE_INFO("a-o","ALL","TEST compare util flush fail");

	return 0;
}

