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
	int i,j,ssd_flag;
	int  SSD_THREAD=300;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
	void *tret;
	int TIMES;
 	
	SSD_CASE_INFO("22-01","ALL","TEST the multi thread  write delete one id");

        SSD_CASE_INFO("22-01","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("12-03","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("22-01","01","OPEN THE SSD DEVICE");
        } 
  
    
	SSD_CASE_INFO("22-01","02","gen randon one channel id");
    	TIMES=10;
    	gen_order_id(TIMES,1);  //gen the id 44*20
    	gen_randm_id(TIMES);
 
 
	
	SSD_CASE_INFO("22-01","03","every thread  to write delelte one id");
    	SSD_THREAD=300;
    	struct thread_coeff para;
    	para.start_id=store_write[0];
    	para.step=1;
    	para.start_num=0;
    	para.id_num=10;
    
    	for (i = 0; i < SSD_THREAD; i++) {
		j=rand()%2;
		if(j==0){
		  
		if (pthread_create(&tidp[i], NULL, thr_fn_write, &para) != 0) {
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("write",i );
                        SSD_ERR("22-01","03","cant not creat thread write");
			}
		}else{
                if (pthread_create(&tidp[i], NULL, thr_fn_delete, &para) != 0) {
                        ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("delete",i );
                        SSD_ERR("22-01","03","cant not creat thread delelte");
                        }
		}
	}
	
	
 
	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("join",i );
                        SSD_ERR("22-01","03","cant not join thread d");		
}
 
	}
    
	printf("case 22-01-04: ssd_close\n");	
     	SSD_CASE_INFO("22-01","04","ssd_close");
	SSD_close(0);

	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("22-01","ALL","TEST the multi thread  write delete one id test pass");
	else
		printf("test case 22-01:  multi-thread write delete one id case FAIL!!!\n");

	return 0;
}

