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
	int NUM, SSD_THREAD=100;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
	void *tret;
	int TIMES;
    	BlockId id;
    	uint32_t len = 0; 
    	const int BUFFSIZE = BT_DATA_SIZE;  //2m或者10m，这里申请了10

	char*buff = (char *)malloc(BUFFSIZE*sizeof(char));
	
	if (NULL == buff) {
		printf("ERROR! can not malloc buff\n");
		ret = SSD_TEST_ERROR;
        exit(-1);
	}
    
    

	SSD_CASE_INFO("14-02","ALL","TEST the multi thread each for 44 channel");

        SSD_CASE_INFO("14-02","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("14-02","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("14-02","01","OPEN THE SSD DEVICE");
        }  


	SSD_CASE_INFO("14-02","02","gen randon 44 channel id");
		
	SSD_THREAD=100;
		
	NUM =45;
     	TIMES=SSD_THREAD*NUM;
     	gen_order_id(TIMES,1);  //gen the id 45*100

   
   
	SSD_CASE_INFO("14-02","03","write randon 44 channel id"); 
    	for (i=0;i<TIMES;i++){
        		id=store_write[i];
                len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
    			write_process(id,buff,len,SSD_OK);
    	}
 
 
 
     	SSD_CASE_INFO("14-02","03","delete one thread for all channel");
		struct thread_coeff para[100];
     	para[0].start_id=store_write[0];
     	para[0].step=1;
     	para[0].start_num=0;
     	para[0].id_num=NUM;

	for(i=1;i<SSD_THREAD;i++)
	{
		
	para[i].start_id=store_write[0];
        para[i].step=1;
        para[i].start_num=para[i-1].start_num+NUM;
        para[i].id_num=NUM;
	}


     
	 for (i = 0; i < SSD_THREAD; i++) {
		  
		if (pthread_create(&tidp[i], NULL, thr_fn_delete, &para[i]) != 0) {
						
			ret = SSD_TEST_ERROR;
						
			SSD_CYCLE_INFO("delete",i );
                        SSD_ERR("14-02","03","cant not creat thread delete");
			}
           		
	} 
	
	
	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
						
			ret = SSD_TEST_ERROR;
						
			SSD_CYCLE_INFO("join",i );
                        SSD_ERR("14-02","03","cant not join thread d");
		}
 
	}
	
	
		SSD_CASE_INFO("14-02","04","ssd_close");
		SSD_close(0);

	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("14-02","ALL","TEST the multi thread each for 44 channel test pass");
	else
		SSD_CASE_INFO("14-02","ALL","TEST the multi thread each for 44 channel test fail");

	return 0;
}

