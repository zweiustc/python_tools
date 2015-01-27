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
	int i,ssd_flag,len;
	int  SSD_THREAD=44;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
	void *tret;
	int TIMES;
    	
	BlockId id,next_id;
 
  
 
	SSD_CASE_INFO("20","ALL","TEST write read delete the whole SSD");

        SSD_CASE_INFO("20","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("20","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("20","01","OPEN THE SSD DEVICE");
        } 
 
 
	SSD_CASE_INFO("20","02","gen random one channel id");
    	TIMES=1600;//gen 10m*1600 times
    	gen_first_id(&id);
    	for(i=0;i<TIMES;i++)
    	{
       		store_write[i]=id;
       		store_write_temp[i]=id;
       		len=1;
       		gen_next_id(id,&next_id,len,44);
       		id=next_id;
     	}
	gen_randm_id(TIMES);
	
	SSD_CASE_INFO("20","03","40 thread write one channel");
    	SSD_THREAD=40;
    	struct thread_coeff para[44];
    	para[0].start_id=store_write[0];
    	para[0].step=1;
    	para[0].start_num=0;
    	para[0].id_num=40;
	for(i=1;i<40;i++)
	{
	para[i].start_id=store_write[0];
        para[i].step=1;
        para[i].start_num=para[i-1].start_num+40;
        para[i].id_num=40;
	}  
    
    	for (i = 0; i < SSD_THREAD; i++) {
		  
		if (pthread_create(&tidp[i], NULL, thr_fn_write, &para[i]) != 0) {
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
	
	
	SSD_CASE_INFO("20","04","40 thread read one channel");
    	for (i = 0; i < SSD_THREAD; i++) {
		  
		if (pthread_create(&tidp[i], NULL, thr_fn_read, &para[i]) != 0) {
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("read",i );
                        SSD_ERR("20","04","cant not creat thread read");
			}
      	}
	

	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("join",i );
                        SSD_ERR("20","04","cant not join thread d");
		}

    	}
    
	SSD_CASE_INFO("20","05","40 thread delete one channel");
	
    	for (i = 0; i < SSD_THREAD; i++) {
		  
		if (pthread_create(&tidp[i], NULL, thr_fn_delete, &para[i]) != 0) {
			ret = SSD_TEST_ERROR;
		
			SSD_CYCLE_INFO("delete",i );
                        SSD_ERR("20","05","cant not creat thread delete");		
		}
    	}
	

	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("join",i );
                        SSD_ERR("20","05","cant not join thread d");
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

