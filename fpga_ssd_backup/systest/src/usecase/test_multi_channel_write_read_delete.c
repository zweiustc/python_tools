#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include "../../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../../include/test_usecase.h"
#include "../../include/test_macro.h"
 

/*================================หตร๗-==================*/
//para 1: 0:one thread for one channel 1:one thread for all channel 2:all thread for one channel
//para 2: the thread num
//para 3: the total write read delete times

//para 4: write the little num
//para 5: write the big num

//para 6: read little num
//para 7: read big num

//para 8: delete little num
//para 9: delete big num

//para 10: flush  num

//para 11: util   num
int main(int argc,char *argv[])
{
	int i,ssd_flag,len,NUM;
	int  SSD_THREAD,MODE,step;
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	//pthread_t tidp[SSD_THREAD];
	void *tret;
	int TIMES;
    int write_num_1,write_num_2,read_num_1,read_num_2,delete_num_1,delete_num_2,flush_num,util_num;
	int operatin_time_for_one_thread;
	BlockId id,next_id;
 
  
 
	SSD_CASE_INFO("w-r-d","ALL","TEST write,read ,delete");

	SSD_CASE_INFO("w-r-d","01","TEST the input para");
	if(argc !=12){
                SSD_CASE_INFO("w-r-d","01","wrong input para num");
		return -1;
        }

	//the whole para
	MODE=atoi(argv[1]);
	if(MODE < 0 || MODE >2 ){
		SSD_CASE_INFO("w-r-d","01","wrong input para mode");
                return -1;
	}


	SSD_THREAD = atoi(argv[2]);
	if(SSD_THREAD <= 0){
                SSD_CASE_INFO("w-r-d","01","wrong input para thread");
                return -1;
        }
	pthread_t tidp[SSD_THREAD];

	operatin_time_for_one_thread = atoi(argv[3]);
        if(operatin_time_for_one_thread <= 0 ){
                SSD_CASE_INFO("w-r-d","01","wrong input para operation time");
                return -1;
        }

	//the write para

	write_num_1 = atoi(argv[4]);
        if(write_num_1< 0){
                SSD_CASE_INFO("w-r-d","01","wrong input para write number num1");
                return -1;
        }

	write_num_2 = atoi(argv[5]);
        if(write_num_2 < 0){
                SSD_CASE_INFO("w-r-d","01","wrong input para write number num2");
                return -1;
        }


	//the read para
	read_num_1 = atoi(argv[6]);
        if(read_num_1 < 0){
                SSD_CASE_INFO("w-r-d","01","wrong input para read num1");
                return -1;
        }

	read_num_2 = atoi(argv[7]);
        if(read_num_2 < 0){
                SSD_CASE_INFO("w-r-d","01","wrong input para read num2");
                return -1;
        }
	

	//the delete para
	delete_num_1 = atoi(argv[8]);
        if(delete_num_1 < 0){
                SSD_CASE_INFO("w-r-d","01","wrong input para delete number num1");
                return -1;
        }

	delete_num_2 = atoi(argv[9]);
        if(delete_num_2 < 0){
                SSD_CASE_INFO("w-r-d","01","wrong input para delete number num2");
                return -1;
        }

	
	//the flush para
        flush_num = atoi(argv[10]);
        if(flush_num < 0){
                SSD_CASE_INFO("w-r-d","01","wrong input para delete number num1");
                return -1;
        }


	//the util para
        util_num = atoi(argv[11]);
        if(util_num < 0){
                SSD_CASE_INFO("w-r-d","01","wrong input para delete number num1");
                return -1;
        }



	if(delete_num_2 < delete_num_1 || write_num_2 < write_num_1 || read_num_2 < read_num_1 ){
		SSD_CASE_INFO("w-r-d","01","wrong input para delete number num2");
                return -1;
        }

	
	//open the device
        SSD_CASE_INFO("w-r-d","02","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("w-r-d","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("w-r-d","01","OPEN THE SSD DEVICE");
        } 
 
	// begin the operation
	SSD_CASE_INFO("w-r-d","03","begin the thread to test");
	
        id.m_nLow  =0;
        id.m_nHigh =0x07;

	struct thread_coeff para[SSD_THREAD];

	if(MODE == 0){  // one thread for one channel
		
		
		if(SSD_THREAD >44){
    	step=SSD_THREAD;
		}
		else{
		step =44;
		}

		para[0].start_id=id;
		para[0].step=step;
    	para[0].start_num=0;
    	para[0].id_num=operatin_time_for_one_thread;
	
	para[0].write_num_1 = write_num_1;
	para[0].write_num_2 = write_num_2;
	para[0].read_num_1  =read_num_1;
	para[0].read_num_2  =read_num_2;
	para[0].delete_num_1=delete_num_1;
	para[0].delete_num_2=delete_num_2;
	para[0].flush_num   =flush_num;
	para[0].util_num    =util_num;
	for(i=1;i<SSD_THREAD;i++)
		{
			para[i].start_id.m_nLow  = para[i-1].start_id.m_nLow + 1;
			para[i].start_id.m_nHigh = para[i-1].start_id.m_nHigh ;
        	para[i].step=step;
        	para[i].start_num=0;
        	para[i].id_num=operatin_time_for_one_thread;
			para[i].write_num_1 = write_num_1;
        	para[i].write_num_2 = write_num_2;
        	para[i].read_num_1  =read_num_1;
        	para[i].read_num_2  =read_num_2;
        	para[i].delete_num_1=delete_num_1;
        	para[i].delete_num_2=delete_num_2;
        	para[i].flush_num   =flush_num;
        	para[i].util_num    =util_num;
	
		}
	}

	
        if(MODE == 1){

        para[0].start_id=id;
        para[0].step=1;
        para[0].start_num=0;
        para[0].id_num=operatin_time_for_one_thread;

        para[0].write_num_1 = write_num_1;
        para[0].write_num_2 = write_num_2;
        para[0].read_num_1  =read_num_1;
        para[0].read_num_2  =read_num_2;
        para[0].delete_num_1=delete_num_1;
        para[0].delete_num_2=delete_num_2;
        para[0].flush_num   =flush_num;
        para[0].util_num    =util_num;
        for(i=1;i<SSD_THREAD;i++)
        	{
                para[i].start_id.m_nLow  = para[i-1].start_id.m_nLow + operatin_time_for_one_thread*write_num_2;
                para[i].start_id.m_nHigh = para[i-1].start_id.m_nHigh ;
                para[i].step=1;
                para[i].start_num=0;
                para[i].id_num=operatin_time_for_one_thread;
                para[i].write_num_1 = write_num_1;
                para[i].write_num_2 = write_num_2;
                para[i].read_num_1  =read_num_1;
                para[i].read_num_2  =read_num_2;
                para[i].delete_num_1=delete_num_1;
                para[i].delete_num_2=delete_num_2;
                para[i].flush_num	=flush_num;
                para[i].util_num	=util_num;
        
        	} 
	}


        if(MODE == 2){

		if(SSD_THREAD >44){
        step=SSD_THREAD;
        }
        else{
        step =44;
        }

        para[0].start_id=id;
        para[0].step=step;
        para[0].start_num=0;
        para[0].id_num=operatin_time_for_one_thread;

        para[0].write_num_1 = write_num_1;
        para[0].write_num_2 = write_num_2;
        para[0].read_num_1  =read_num_1;
        para[0].read_num_2  =read_num_2;
        para[0].delete_num_1=delete_num_1;
        para[0].delete_num_2=delete_num_2;
        para[0].flush_num=flush_num;
        para[0].util_num=util_num;
        for(i=1;i<SSD_THREAD;i++)
        	{
                para[i].start_id.m_nLow  = para[i-1].start_id.m_nLow + step*write_num_2*operatin_time_for_one_thread;
                para[i].start_id.m_nHigh = para[i-1].start_id.m_nHigh ;
                para[i].step=step;
                para[i].start_num=0;
                para[i].id_num=operatin_time_for_one_thread;
                para[i].write_num_1 = write_num_1;
                para[i].write_num_2 = write_num_2;
                para[i].read_num_1  =read_num_1;
                para[i].read_num_2  =read_num_2;
                para[i].delete_num_1=delete_num_1;
                para[i].delete_num_2=delete_num_2;
                para[i].flush_num=flush_num;
                para[i].util_num=util_num;
        
        	} 
  	}
    
    			
	for (i = 0; i < SSD_THREAD; i++) {
		  
				if (pthread_create(&tidp[i], NULL, thr_fn_operation, &para[i]) != 0) {
					ret = SSD_TEST_ERROR;
					SSD_CYCLE_INFO("fn_all_operation",i );
                        		SSD_ERR("w-r-d","03","cant not creat thread");
					}
    			
	}
	

			
	for (i = 0; i < SSD_THREAD; i++) {
				if (pthread_join(tidp[i], &tret) != 0) { 
					ret = SSD_TEST_ERROR;
					SSD_CYCLE_INFO("join",i );
                        		SSD_ERR("w-r-d","03","cant not join thread ");
					}
	}
 
		
	
	
          
        
	SSD_CASE_INFO("w-r-d","04","ssd_close");	
	SSD_close(0);

	if (SSD_TEST_OK == ret)
		printf("test case w-r-d:  multi-thread write read delete flush util case PASS!!!\n");
	else
		printf("test case w-r-d:  multi-thread write read delete flush util case FAIL!!!\n");

	return 0;
}

