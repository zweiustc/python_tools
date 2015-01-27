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
	int NUM, SSD_THREAD=300;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
	void *tret;
	int TIMES;
	
	BlockId id;
        uint32_t len;
        const int BUFFSIZE = BT_DATA_SIZE;  // ?a¨¤?¨¦¨º??¨¢?10
        char * buff = (char *)malloc(BUFFSIZE*sizeof(char));
        if (NULL == buff) {
                printf("ERROR! can not malloc buff\n");
                ret = SSD_TEST_ERROR;
        exit(-1);
        }
 	
	SSD_CASE_INFO("25-03","ALL","TEST the multi thread  write read for one channel");

        SSD_CASE_INFO("25-03","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("25-03","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("25-03","01","OPEN THE SSD DEVICE");
        } 
  
    
	SSD_CASE_INFO("25-03","02","gen randon one channel id");
		
	NUM=2;
		
	SSD_THREAD=300;
    	TIMES=NUM*SSD_THREAD;
    	gen_order_id(TIMES,44);  //gen the id 44*20
	 for (i=0;i<TIMES/2;i++){
                id=store_write[i];
                len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
                write_process(id,buff,len,SSD_OK);
        } 
 
	
	SSD_CASE_INFO("25-03","03","every thread  to write read one id");
    	SSD_THREAD=300;
   	struct thread_coeff para_1[300];
        para_1[0].start_id=store_write[0];
        para_1[0].step=1;
        para_1[0].start_num=0;
        para_1[0].id_num=NUM;


        for(i=1;i<300;i++)
        {
        para_1[i].start_id=store_write[0];
        para_1[i].step=1;
        para_1[i].start_num=para_1[i-1].start_num+NUM;
        para_1[i].id_num=NUM;


        } 
    	for (i = 0; i < SSD_THREAD; i++) {
		
		if(i>149){
		  
		if (pthread_create(&tidp[i], NULL, thr_fn_write, &para_1[i]) != 0) {
						ret = SSD_TEST_ERROR;
						SSD_CYCLE_INFO("write",i );
                        SSD_ERR("25-03","03","cant not creat thread write");
			}
		}else{
                if (pthread_create(&tidp[i], NULL, thr_fn_read, &para_1[i]) != 0) {
                        ret = SSD_TEST_ERROR;
						SSD_CYCLE_INFO("delete",i );
                        SSD_ERR("25-03","03","cant not creat thread read");
                        }
		}
	}
	
	
 
	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("join",i );
                        SSD_ERR("25-03","03","cant not join thread d");		
}
 
	}
    
     	SSD_CASE_INFO("25-03","04","ssd_close");
		SSD_close(0);

	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("25-03","ALL","TEST the multi thread  write read one channel test pass");
	else
		SSD_CASE_INFO("25-03","ALL","TEST the multi thread  write read one channel  test fail");

	return 0;
}

