#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include "../../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../../include/test_macro.h"
#include "../../include/test_tools.h"

//char ssd_name[SSD_MAX_CHANNEL][16];
 

int main(void)
{
	int i,j,ssd_flag;
	int  SSD_THREAD=44;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	pthread_t tidp[SSD_THREAD];
	void *tret;
	int TIMES;
    	uint32_t len = 0;
        BlockId id,next_id; 
    	const int BUFFSIZE = 100 * 1024 * 1024;  //2m或者10m，这里申请了10
	char *buff = (char *)malloc(BUFFSIZE*sizeof(char));	
	if (NULL == buff) {
		printf("ERROR! can not malloc buff\n");
		ret = SSD_TEST_ERROR;
        exit(-1);
	}
    
	    
 	
	SSD_CASE_INFO("17","ALL","TEST the multi thread  all for one channel");

        SSD_CASE_INFO("17","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("17","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("17","01","OPEN THE SSD DEVICE");
        }
 
	SSD_CASE_INFO("17","02","write TIMES");
    	TIMES=44*50;
    	gen_order_id(TIMES,1);  //gen the id 44*20
    	gen_randm_id(TIMES);
    	for (i=0;i<TIMES/2;i++){
    		id=store_write[i];
                len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
    		write_process(id,buff,len,SSD_OK);
     	}
	struct BlockId write_id[44];
	struct BlockId read_id[44];
	struct BlockId delete_id[44];
    	gen_next_id(id,&write_id[0],1,1);
        read_id[0]=write_id[0];
	delete_id[0]=write_id[0];    
 
       	SSD_CASE_INFO("17","03","each channel for ab and nor operation");	
    	SSD_THREAD=44;

    	struct thread_coeff para_1[44];
    	para_1[0].start_id=store_write[0];
    	para_1[0].step=1;
    	para_1[0].start_num=0;
    	para_1[0].id_num=40;
   
    	struct thread_coeff para_2[44];
    	para_2[0].start_id=store_write[0];
    	para_2[0].step=1;
    	para_2[0].start_num=600;
    	para_2[0].id_num=40;
                      
    	struct thread_coeff para_3[44];
    	para_3[0].start_id=store_write[0];
    	para_3[0].step=1;
    	para_3[0].start_num=1200;
    	para_3[0].id_num=40;
	
	for(i=1;i<44;i++)
	{
	para_1[i].start_id=store_write[0];
        para_1[i].step=1;
        para_1[i].start_num=para_1[i-1].start_num+1;
        para_1[i].id_num=40;

        para_2[i].start_id=store_write[0];
        para_2[i].step=1;
        para_2[i].start_num=para_2[i-1].start_num+1;
        para_2[i].id_num=40;

        para_3[i].start_id=store_write[0];
        para_3[i].step=1;
        para_3[i].start_num=para_3[i-1].start_num+1;
        para_3[i].id_num=40;
	}	
	
	for(i=1;i<44;i++)
	{
	gen_next_id(write_id[i-1],&next_id,1,1);
	write_id[i] = next_id;
        read_id[i]  = next_id;
	delete_id[i]= next_id;
	}	
	   
    
	for (i = 0; i < SSD_THREAD; i++) {
		j=rand();
           	if(j%6==1){
                	if (pthread_create(&tidp[i], NULL, thr_fn_write, &para_1[i]) != 0) {
				ret = SSD_TEST_ERROR;
				SSD_CYCLE_INFO("write_nor",i );
                       		SSD_ERR("17","03","cant not creat thread write_ab");
			       }
		  		
		  			  		
		}if(j%6==2){
			if (pthread_create(&tidp[i], NULL, thr_fn_write_ab, &write_id[i]) != 0) {
				ret = SSD_TEST_ERROR;
				SSD_CYCLE_INFO("write_ab",i );
                                SSD_ERR("17","03","cant not creat thread write_ab");
			       }
		  			  		
		}else if(j%6==3){
		  	if (pthread_create(&tidp[i], NULL, thr_fn_read, &para_2[i]) != 0) {
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("read_nor",i );
                           
			SSD_ERR("17","03","cant not creat thread read_nor");
			}
		  		
		}else if(j%6==4){
			if (pthread_create(&tidp[i], NULL, thr_fn_read_ab, &read_id[i]) != 0) {
				ret = SSD_TEST_ERROR;
				SSD_CYCLE_INFO("read_ab",i );
                                SSD_ERR("17","03","cant not creat thread read_ab");
			       		}	
			   
		}else if(j%6==5){
                    
			if (pthread_create(&tidp[i], NULL, thr_fn_delete, &para_3[i]) != 0) {
				       ret = SSD_TEST_ERROR;
					SSD_CYCLE_INFO("delete_nor",i );
                                	SSD_ERR("17","03","cant not creat thread delete_nor");
			       }
			  		  		
		}else {
			if (pthread_create(&tidp[i], NULL, thr_fn_delete_ab, &delete_id[i]) != 0) {
				       ret = SSD_TEST_ERROR;
					SSD_CYCLE_INFO("delete_ab",i );
                                	SSD_ERR("17","03","cant not creat thread delete_ab");
			       }
		  				  		
		}
		  	 
	
 
	}
	
	
	for (i = 0; i < SSD_THREAD; i++) {
		if (pthread_join(tidp[i], &tret) != 0) { 
			ret = SSD_TEST_ERROR;
			SSD_CYCLE_INFO("join",i );
                        SSD_ERR("17","03","cant not join thread d");
		}
 
    	} 
	
	

	SSD_CASE_INFO("17","04","ssd_close");     
	SSD_close(0);

	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("17","ALL","TEST the multi thread  all for one channel test pass");
	else
		SSD_CASE_INFO("17","ALL","TEST the multi thread  all for one channel test fail");

	return 0;
}
