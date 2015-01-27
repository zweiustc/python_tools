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
	int NUM, SSD_THREAD=50;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
	void *tret;
	int TIMES;
  
  
   
	SSD_CASE_INFO("12-03","ALL","TEST the multi thread  all for one channel");

        SSD_CASE_INFO("12-03","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("12-03","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("12-03","01","OPEN THE SSD DEVICE");
        }

 
	SSD_CASE_INFO("12-03","02","gen random one channel id");
    	SSD_THREAD=50;
		
	NUM=10;
		
	TIMES=SSD_THREAD*NUM;
    	gen_order_id(TIMES,44);  //gen the id 44*20
    	gen_randm_id(TIMES);
 
 
	

	SSD_CASE_INFO("12-03","03","every thread for one channel to write one channel");	
    	struct thread_coeff para[50];
    	para[0].start_id=store_write[0];
    	para[0].step=1;
    	para[0].start_num=0;
    	para[0].id_num=NUM;
	for(i=1;i<50;i++)
	{
	para[i].start_id=store_write[0];
        para[i].step=1;
        para[i].start_num=para[i-1].start_num+NUM;
        para[i].id_num=NUM;
	}
    
    	for (i = 0; i < SSD_THREAD; i++) {
		  
		if (pthread_create(&tidp[i], NULL, thr_fn_write, &para[i]) != 0) {
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("write",i );
                        
			SSD_ERR("12-03","03","cant not creat thread write");
			}
	}
	
	
 
	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("join",i );
                        
			SSD_ERR("12-03","03","cant not join thread d");
		}
 
	}



    
 	SSD_CASE_INFO("12-03","04","ssd_close"); 
	SSD_close(0);

	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("12-03","ALL","TEST the multi thread  all for one channel test pass");
	else
		SSD_CASE_INFO("12-03","ALL","TEST the multi thread  all for one channel test fial");

	return 0;
}

