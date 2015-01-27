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
	int i,ssd_flag,len,NUM;
	int  SSD_THREAD=44;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
	void *tret;
	int TIMES;
    	
	BlockId id,next_id;
 
  
 
	SSD_CASE_INFO("20","ALL","TEST write more than whole SSD,the data is right or not");

        SSD_CASE_INFO("20","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("20","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("20","01","OPEN THE SSD DEVICE");
        } 
 
	
	SSD_CASE_INFO("20","03","40 thread write one channel");
    	SSD_THREAD=44;
	NUM=1601;
        id.m_nLow  =0;
        id.m_nHigh =0x07;

    	struct thread_coeff para[44];
    	para[0].start_id=id;
    	para[0].step=44;
    	para[0].start_num=0;
    	para[0].id_num=NUM;
	for(i=1;i<44;i++)
	{
	para[i].start_id.m_nLow  = para[i-1].start_id.m_nLow + 1;
	para[i].start_id.m_nHigh = para[i-1].start_id.m_nHigh ;
        para[i].step=44;
        para[i].start_num=0;
        para[i].id_num=NUM;
	}  
    
    	for (i = 0; i < SSD_THREAD; i++) {
		  
		if (pthread_create(&tidp[i], NULL, thr_fn_compare_divide, &para[i]) != 0) {
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("write",i );
                        SSD_ERR("20","03","cant not creat thread write");
		}
    	}
	

	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("join",i );
                        SSD_ERR("20","03","cant not join thread d");
		}
 
	}
	
	
          
        
		SSD_CASE_INFO("20","05","ssd_close");	
	     	SSD_close(0);

	if (SSD_TEST_OK == ret)
		printf("test case 20: one-channel multi-thread write read delete full channel case PASS!!!\n");
	else
		printf("test case 20: one_channel multi-thread write read delete full channel case FAIL!!!\n");

	return 0;
}

