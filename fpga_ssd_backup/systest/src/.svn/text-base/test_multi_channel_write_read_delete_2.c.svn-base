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
	int  SSD_THREAD=100;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
	void *tret;
	int TIMES;
    	const int BUFFSIZE = BT_DATA_SIZE;  //2m或者10m，这里申请了10

	char *buff = (char *)malloc(BUFFSIZE*sizeof(char));
	if (NULL == buff) {
		printf("ERROR! can not malloc buff\n");
		ret = SSD_TEST_ERROR;
        exit(-1);
	}
    
	SSD_CASE_INFO("16-02","ALL","TEST the multi thread  every for both write read delete");

        SSD_CASE_INFO("16-02","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("16-02","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("16-02","01","OPEN THE SSD DEVICE");
        }    
       
	 
	SSD_CASE_INFO("16-02","02","random write TIMES"); 
        TIMES=6000; 
    	gen_order_id(TIMES,1);  //gen the id 44*20
        gen_randm_id(TIMES);                     
 
	SSD_CASE_INFO("16-02","03","each thread for write read delete");	
    	SSD_THREAD=100;
    	struct thread_coeff para[100];
    	para[0].start_id=store_write[0];
    	para[0].step=1;
    	para[0].start_num=0;
    	para[0].id_num=60;
	for(i=1;i<100;i++)
	{
	para[i].start_id=store_write[0];
        para[i].step=1;
        para[i].start_num= para[i-1].start_num+60;
        para[i].id_num=60;
	}

    
  
	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_create(&tidp[i], NULL, thr_fn_write_read_delete, &para[i]) != 0) {
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("multi",i );
                        SSD_ERR("16-02","03","cant not creat thread multi");
			}
                 
		  }

	
	
	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("join",i );
                        SSD_ERR("16-02","03","cant not join thread d");
		}
      	}
        
	SSD_CASE_INFO("16-02","04","ssd_close");	
      	SSD_close(0);

	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("16-02","ALL","TEST the multi thread  every for both write read delete test pass");
	else
		SSD_CASE_INFO("16-02","ALL","TEST the multi thread  every for both write read delete test fail");

	return 0;
}

