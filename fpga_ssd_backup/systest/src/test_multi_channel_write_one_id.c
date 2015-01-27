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
	int i;
	int  SSD_THREAD=300;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
	void *tret;
	int TIMES;
  
    	printf("case 21-01-01: ssd_open()\n");
	if ( SSD_open(0)!= SSD_OK) {
		printf("\tERROR!\n");
		printf("\tcase21-01-01, excepted: %d\n", SSD_OK);
		ret = SSD_OPEN_ERROR;
        exit(-1);
	} else {
		printf("case 21-01-01: SSD_OPEN returns successfully\n");
	}
  
    
    	printf("case 21-01-02:gen random one channel id\n");
    	TIMES=10;
    	gen_order_id(TIMES,1);  //gen the id 44*20
    	gen_randm_id(TIMES);
 
 
	
	printf("case 21-01-03: every thread for one channel to write one channel  \n");	
	
    	SSD_THREAD=300;
    	struct thread_coeff para;
    	para.start_id=store_write[0];
    	para.step=1;
    	para.start_num=0;
    	para.id_num=10;
    
    	for (i = 0; i < SSD_THREAD; i++) {
		  
		printf("case 21-01-03-%d:  write with Right param\n", i);
		if (pthread_create(&tidp[i], NULL, thr_fn_write, &para) != 0) {
			printf("\tERROR! case 21-01-03-%d: can not creat thread write\n", i);
			ret = SSD_TEST_ERROR;
                	exit(-1);
			}
	}
	
	
 
	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
			printf("ERROR! case 21-01-03-%d: can not join with thread\n", i);
			ret = SSD_TEST_ERROR;
            exit(-1);
		}
 
	}
/*	
     printf("case 12-3-03: delete tesing\n");//to do :use the SSD_ftw()
     id=temp_id;
     test_times=SSD_THREAD;
      for (i=0;i<=test_times;i++){
       printf("case 12-3-03: delete the %d times \n",i);
       delete_process(id,SSD_OK);
       gen_same_channel_id(id,next_id);
       id=next_id;
     }



*/
    
	printf("case 21-01-04: ssd_close\n");	
/*	if ( SSD_close() != SSD_OK) {
	  	printf("\tERROR!\n");
	  	ret = SSD_CLOSE_ERROR;
        	exit(-1);
	} else {
	  	printf("case 12-03-03: SSD_CLOSE returns successfully\n");
	}
*/      SSD_close(0);

	if (SSD_TEST_OK == ret)
		printf("test case 21-01: multi-channel multi-thread write case PASS!!!\n");
	else
		printf("test case 21-01: multi_channel multi-thread write case FAIL!!!\n");

	return 0;
}

