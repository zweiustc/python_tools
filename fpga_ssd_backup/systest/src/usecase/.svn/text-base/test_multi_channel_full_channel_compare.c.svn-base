#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include "../../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../../include/test_usecase.h"
#include "../../include/test_macro.h"
 

 


int main(int argc,char *argv[])
{
	int i,ssd_flag,len,NUM;
	int  SSD_THREAD;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
	void *tret;
	int TIMES;
    	int percentage;
	BlockId id,next_id;
 
  
 
	SSD_CASE_INFO("w-r-d","ALL","TEST write whole SSD ,read whole SSD,delete whole SSD");

        SSD_CASE_INFO("w-r-d","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("w-r-d","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("w-r-d","01","OPEN THE SSD DEVICE");
        } 
 
	
	SSD_CASE_INFO("w-r-d","03","44 thread write read delete");
	
	if(argc !=2){
		SSD_CASE_INFO("w-r-d","03","wrong input para num");
	}
	else{
		percentage=atoi(argv[1]);
		if(percentage <= 0 || percentage > 200){
			SSD_CASE_INFO("w-r-d","03","wrong input para range");
		}
		else{
			SSD_THREAD=44;
			NUM=1600*percentage/100;
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
					SSD_CYCLE_INFO("fn_compare_divide",i );
                        		SSD_ERR("w-r-d","03","cant not creat thread");
					}
    			}
	

			for (i = 0; i < SSD_THREAD; i++) {
				if (pthread_join(tidp[i], &tret) != 0) { 
					ret = SSD_TEST_ERROR;
					SSD_CYCLE_INFO("join",i );
                        		SSD_ERR("20","03","cant not join thread d");
					}
			}
 
		}
	
	}
          
        
	SSD_CASE_INFO("w-r-d","04","ssd_close");	
	SSD_close(0);

	if (SSD_TEST_OK == ret)
		printf("test case w-r-d: 44 multi-thread write read delete full channel case PASS!!!\n");
	else
		printf("test case w-r-d: 44 multi-thread write read delete full channel case FAIL!!!\n");

	return 0;
}

