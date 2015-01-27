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
 
  
 
	SSD_close(0);
	
	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("a-o","ALL","TEST compare util flush pass");
	else
		SSD_CASE_INFO("a-o","ALL","TEST compare util flush fail");

	return 0;
}

