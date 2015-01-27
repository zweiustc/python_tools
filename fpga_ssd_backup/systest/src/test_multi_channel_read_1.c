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
	int  NUM,SSD_THREAD=44;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
	void *tret;
	int TIMES;
	BlockId id;
	uint32_t len = 0; 
    	
	const int BUFFSIZE = BT_DATA_SIZE;  // ’‚¿Ô…Í«Î¡À10
	char * buff = (char *)malloc(BUFFSIZE*sizeof(char));	
	if (NULL == buff) {
		printf("ERROR! can not malloc buff\n");
		ret = SSD_TEST_ERROR;
        exit(-1);
	}
    
    
   
	SSD_CASE_INFO("13-01","ALL","TEST the multi thread read each for one channel");

        SSD_CASE_INFO("13-01","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("13-01","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("13-01","01","OPEN THE SSD DEVICE");
        }
 


	SSD_CASE_INFO("13-01","02","gen randon one channel id");
    	NUM=30;
    	SSD_THREAD=44;
		TIMES=SSD_THREAD*NUM;
    	gen_order_id(TIMES,1);  //gen the id 44*20

        SSD_CASE_INFO("13-01","03","write randon TIMES");       
    	for (i=0;i<TIMES;i++){
        	
				id=store_write[i];
                len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
    		
				write_process(id,buff,len,SSD_OK);
    	}
 
	SSD_CASE_INFO("13-01","03","read one thread for one channel"); 
		struct thread_coeff para[44];
    	para[0].start_id=store_write[0];
    	para[0].step=SSD_THREAD;
    	para[0].start_num=0;
    	para[0].id_num=NUM;
	for(i=1;i<44;i++)
	{
	para[i].start_id=store_write[0];
        para[i].step=SSD_THREAD;
        para[i].start_num=para[i-1].start_num+1;
        para[i].id_num=NUM;
	}
    
    	for (i = 0; i < SSD_THREAD; i++) {
		  
		if (pthread_create(&tidp[i], NULL, thr_fn_read, &para[i]) != 0) {
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("read",i );
                        SSD_ERR("13-01","03","cant not creat thread read");
			}
	}
	
	
		
	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("join",i );
                        SSD_ERR("13-01","03","cant not join thread d");
		}
 
	}
	
	

	SSD_CASE_INFO("13-01","04","ssd_close");
	SSD_close(0);

	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("13-01","ALL","TEST the multi thread read each for one channel test pass");
	else
		SSD_CASE_INFO("13-01","ALL","TEST the multi thread read each for one channel test fial");

	return 0;
}

