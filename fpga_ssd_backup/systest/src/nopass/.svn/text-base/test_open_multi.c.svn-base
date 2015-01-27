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
	int  SSD_THREAD=150;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
	void *tret;
	int TIMES;
	BlockId id;
	uint32_t len = 0; 
    	
	const int BUFFSIZE = BT_DATA_SIZE;  //2m或者10m，这里申请了10
	char *buff = (char *)malloc(BUFFSIZE*sizeof(char));	
	if (NULL == buff) {
		printf("ERROR! can not malloc buff\n");
		ret = SSD_TEST_ERROR;
        exit(-1);
	}
    
    
	
	SSD_CASE_INFO("01-03","01","multi thread open");	
    	SSD_THREAD=150;
    	struct thread_coeff para_1[150];
    	para_1[0].start_id=store_write[0];
    	para_1[0].step=1;
    	para_1[0].start_num=0;
    	para_1[0].id_num=4;

	for(i=1;i<150;i++)
	{
	para_1[i].start_id=store_write[0];
        para_1[i].step=1;
        para_1[i].start_num=para_1[i-1].start_num+4;
        para_1[i].id_num=4;

	}

    
	for (i = 0; i < 150; i++) {
          
		  if (pthread_create(&tidp[i], NULL, thr_fn_open, NULL) != 0) {
		    		SSD_CYCLE_INFO("open",i );
                        
				SSD_ERR("01-03","01","cant not creat thread open");
			 
				ret = SSD_TEST_ERROR;
	            }
	}
	
	
	for (i = 0; i < 150; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
				SSD_CYCLE_INFO("join",i );
                        
				SSD_ERR("01-03","01","cant not join thread flush");   
				ret = SSD_TEST_ERROR;
		}
      	}


        
	SSD_CASE_INFO("01-03","02","SSD DEVICE CLOSE");
        SSD_close(0);

	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("03-02","ALL","multi thread open test pass");
	else
		SSD_CASE_INFO("03-02","ALL","multi thread open test fail");
	return 0;
}

