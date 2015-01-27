#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include "../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../include/test_tools.h"
#include "../include/test_macro.h"
 

 


int main(void)
{
	int i,ssd_flag;
	int  SSD_THREAD=44,NUM;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
	void *tret;
    	BlockId id;
 
  
 
	SSD_CASE_INFO("a-com","ALL","TEST write the whole SSD 90%");


        SSD_CASE_INFO("w-f","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("a-com","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("a-com","01","OPEN THE SSD DEVICE");
        }
 
 
 
	
	SSD_CASE_INFO("a-com","02","all thread to write the whole ssd");
    	SSD_THREAD=44;
	NUM=1500;
	id.m_nLow  =0;
	id.m_nHigh =0x07;

    	struct thread_coeff para[44];
    	para[0].start_id=id;
    	para[0].step=44;
    	para[0].start_num=0;
    	para[0].id_num=NUM;//10m*1500
	for(i=1;i<44;i++)
	{
		para[i].start_id.m_nLow =  para[i-1].start_id.m_nLow + 1;
        para[i].step=44;
        para[i].start_num=0;
        para[i].id_num=NUM;
	}  
    
    	for (i = 0; i < SSD_THREAD; i++) {
		  
		if (pthread_create(&tidp[i], NULL, thr_fn_compare, &para[i]) != 0) {
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("write",i );
                        SSD_ERR("a-com","02","cant not creat thread write");
		}
    	}
	

	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("join",i );
                        SSD_ERR("a-com","02","cant not join thread d");
		}
 
	}
	
     	SSD_CASE_INFO("a-com","03","ssd_close");
	SSD_close(0);

	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("a-com","ALL","TEST compare the whole SSD 90 per test pass");
	else
		SSD_CASE_INFO("a-com","ALL","TEST compare the whole SSD 90 per test fail");

	return 0;
}

