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
	int  NUM,SSD_THREAD=440;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
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
 
 
 
	
	SSD_CASE_INFO("a-o","02","all thread to write the whole ssd");
    	
	SSD_THREAD=440;
	NUM=200;
	id.m_nLow  =0;
	id.m_nHigh =0x07;

    	struct thread_coeff para[440];
    	para[0].start_id=id;
    	para[0].step=440;
    	para[0].start_num=0;
    	para[0].id_num=NUM;//10m*1500
	for(i=1;i<440;i++)
	{
	para[i].start_id.m_nLow =   para[i-1].start_id.m_nLow + 1;
	para[i].start_id.m_nHigh =  para[i-1].start_id.m_nHigh ;
        para[i].step=440;
        para[i].start_num=0;
        para[i].id_num=NUM;
	}  
    
    	for (i = 0; i < SSD_THREAD; i++) {
		  
		if (pthread_create(&tidp[i], NULL, thr_fn_all_operation_random, &para[i]) != 0) {
						
			ret = SSD_TEST_ERROR;
						
			SSD_CYCLE_INFO("operation",i );
                        SSD_ERR("a-o","02","cant not creat thread operation");
		}
    	}
	

	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("join",i );
                        SSD_ERR("a-o","02","cant not join thread ");
		}
 
	}
	
     	SSD_CASE_INFO("a-o","03","ssd_close");
	SSD_close(0);

	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("a-o","ALL","TEST compare util flush pass");
	else
		SSD_CASE_INFO("a-o","ALL","TEST compare util flush fail");

	return 0;
}

