#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include "../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../include/test_tools.h"
#include "../include/test_macro.h"

//char ssd_name[SSD_MAX_CHANNEL][16];
 
 
 
 
 

int main(void)
{
	int i,ssd_flag;
	int  SSD_THREAD=300;
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
    
   
	SSD_CASE_INFO("21-03","ALL","TEST the multi thread delete one id");

        SSD_CASE_INFO("21-03","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("21-03","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("21-03","01","OPEN THE SSD DEVICE");
        } 
    
	SSD_CASE_INFO("21-03","02","gen rand one channel id");
    	TIMES=45;
    	gen_order_id(TIMES,1);  //gen the id 44*20
    	gen_randm_id(TIMES);
   
	SSD_CASE_INFO("21-03","03","write TIMES");
    	for (i=0;i<TIMES;i++){
    		id=store_write_temp[i];
                
			len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
        	write_process(id,buff,len,SSD_OK);
    	}
 
 
	
	SSD_CASE_INFO("21-03","04","multi thread delete one id ");
	SSD_THREAD=300;
      	struct thread_coeff para;
      	para.start_id=store_write[0];
      	para.step=1;
      	para.start_num=0;
      	para.id_num=45;
      
	for (i = 0; i < SSD_THREAD; i++) {		  
		if (pthread_create(&tidp[i], NULL, thr_fn_delete, &para) != 0) {
						ret = SSD_TEST_ERROR;
						SSD_CYCLE_INFO("delete",i );
                        SSD_ERR("21-03","04","cant not creat thread delete");
		}
		
	}
			
	
	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
						ret = SSD_TEST_ERROR;
						SSD_CYCLE_INFO("join",i );
                        SSD_ERR("21-03","04","cant not join thread d");
		}
 
	}
	
	
     	SSD_CASE_INFO("21-0","05","ssd_close");
	SSD_close(0);

	if (SSD_TEST_OK == ret)
	
		SSD_CASE_INFO("21-03","ALL","TEST the multi thread delete one id test pass");	
	else
		SSD_CASE_INFO("21-03","ALL","TEST the multi thread delete one id test fail");

	return 0;
}

