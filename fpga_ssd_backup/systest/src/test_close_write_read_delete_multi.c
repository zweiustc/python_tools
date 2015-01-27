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
    
    
	SSD_CASE_INFO("03-03","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("03-03","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("03-03","01","OPEN THE SSD DEVICE");
        } 

    

	SSD_CASE_INFO("03-03","02","random write TIMES"); 
    	TIMES=400;
    	gen_order_id(TIMES,1);  //gen the id 44*20
    	for (i=0;i<TIMES/2;i++){
       		id=store_write[i];
		len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
       		write_process(id,buff,len,SSD_OK);
    	}	
    	
 
 
	
	SSD_CASE_INFO("03-03","03","multi thread write delete util");	
    	SSD_THREAD=150;
    	struct thread_coeff para_1[100];
    	para_1[0].start_id=store_write[0];
    	para_1[0].step=1;
    	para_1[0].start_num=0;
    	para_1[0].id_num=4;

	for(i=1;i<100;i++)
	{
	para_1[i].start_id=store_write[0];
        para_1[i].step=1;
        para_1[i].start_num=para_1[i-1].start_num+4;
        para_1[i].id_num=4;

	}

    
	for (i = 0; i < 150; i++) {
          
	        
		if(i<25){
		  if (pthread_create(&tidp[i], NULL, thr_fn_delete, &para_1[i]) != 0) {
		     	SSD_CYCLE_INFO("delete",i );
				SSD_ERR("03-03","03","cant not creat thread delete");
		     	ret = SSD_TEST_ERROR;
		  }
		}
		else if(i>=25&&i<50){
		  if (pthread_create(&tidp[i], NULL, thr_fn_read, &para_1[i]) != 0) {
		   	 	SSD_CYCLE_INFO("util",i );
                    
				SSD_ERR("03-03","03","cant not creat thread write");
				ret = SSD_TEST_ERROR;
		   }
		}
		else if(i>=50&&i<100){
		if (pthread_create(&tidp[i], NULL, thr_fn_write, &para_1[i]) != 0) {
                                SSD_CYCLE_INFO("util",i );

                                SSD_ERR("03-03","03","cant not creat thread read");
                                ret = SSD_TEST_ERROR;
		   }
		}
		else {
		  if (pthread_create(&tidp[i], NULL, thr_fn_close, NULL) != 0) {
		    		SSD_CYCLE_INFO("delete",i );
                        
				SSD_ERR("03-03","03","cant not creat thread flush");
			 
				ret = SSD_TEST_ERROR;
	            }
	        }	
	}
	
	
	for (i = 0; i < 150; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
				SSD_CYCLE_INFO("join",i );
                        
				SSD_ERR("03-03","03","cant not join thread ");   
				ret = SSD_TEST_ERROR;
		}
      	}


        
	SSD_CASE_INFO("03-03","04","SSD DEVICE CLOSE");

	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("03-03","ALL","multi thread write delete read flush test pass");
	else
		SSD_CASE_INFO("03-03","ALL","multi thread write delete read flush test fail");
	return 0;
}

