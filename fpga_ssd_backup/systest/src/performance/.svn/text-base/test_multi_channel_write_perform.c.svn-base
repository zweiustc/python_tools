#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include "../../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../../include/test_macro.h"
#include "../../include/test_tools.h"

#define MEASURE  10

typedef struct _perform {
    float best;
    float worst;
    float avrg;
} perform_t;    /* the best, worst and averge perorm M/s */

void each_channel_perform(const struct  BlockId id, perform_t *perform)
{
   // char device[16];
    BlockId id_cur=id,next_id;
    int  SSD_THREAD=44;
    int TIMES;
    //char *buff =(char *) malloc(BT_DATA_SIZE * sizeof(char));  ///根据实际情况修改
    uint32_t  len;
    pthread_t tidp[SSD_THREAD];
    void *tret;
  
     
    len=BT_DATA_SIZE;//10*1024*1024;

    int i; /* measure 3 times to pick up the best, worst and average */
    int j; /* 44 thread operations in every measure */
    struct timeval s_time, e_time;

    for (i = 0; i < 3; i++) {
        float temp_perform = 0.0;
        int vol = len/ (1024 * 1024); /*  how many MB to earse*/
		vol=SSD_THREAD*MEASURE*vol; 
 		
        TIMES=44*10;
	
	gen_first_id(&id_cur);
        for(j=0;j<TIMES;j++){
                gen_next_id(id_cur,&next_id,1,1);
                id_cur=next_id;
                store_write_temp[j]=id_cur;
        }
        struct thread_coeff para[44];
        para[0].start_id=store_write[0];
        para[0].step=44;
        para[0].start_num=0;
        para[0].id_num=10;
	for(j=1;j<44;j++)
	{
	para[j].start_id=store_write[0];
        para[j].step=44;
        para[j].start_num=para[j-1].start_num+1;
        para[j].id_num=10;
	}
        
        gettimeofday(&s_time, NULL);
        for (j = 0; j < SSD_THREAD; j++) {
             if (pthread_create(&tidp[j], NULL, thr_fn_write, &para[j]) != 0) {
             	SSD_CYCLE_INFO("write",i );

                SSD_ERR("","","cant not creat thread write");
		}
        //     para.start_num=para.start_num+1;
        }
        for (j = 0; j < SSD_THREAD; j++) {
            if (pthread_join(tidp[j], &tret) != 0) {
            	SSD_CYCLE_INFO("join",i );

                SSD_ERR("","","cant not join thread d");
		}
        }
          
        
        gettimeofday(&e_time, NULL);
        
        
        //==============================以下根据实际情况修改===============

        temp_perform = ( vol)    /* times * volume */
            / ((e_time.tv_sec - s_time.tv_sec) + (e_time.tv_usec - s_time.tv_usec) / 1000000.0);

        if (temp_perform < perform->worst)
            perform->worst = temp_perform;
        else if (temp_perform > perform->best)
            perform->best = temp_perform;
        perform->avrg += temp_perform;
    }
    perform->avrg /= 3.0;

    
}

int main(void)
{
    srand((unsigned)time(NULL));
  //  SSD_MAX_CHANNEL=44
    perform_t perform[1];     /* each channel perform data */
  //  int SSD_MAX_CHANNEL=1;

    BlockId id,next_id;
    enum SSD_TEST_RET_TYPE ret;
    int ssd_flag;  
    /*open the ssd device*/
	   
	SSD_CASE_INFO("w_p","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("w-p","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("w-p","01","OPEN THE SSD DEVICE");
        } 

    /* init the data struct to M/s */
    int channel;
    for (channel = 0; channel < 1; channel++) {
        perform[channel].best = 0.0;
        perform[channel].worst = 9999.99;
        perform[channel].avrg = 0.0;
    }

    /* calculate each channel perform */
	SSD_CASE_INFO("w_p","02","calculate each channel perform");
    gen_first_id(&id);
    for (channel = 0; channel < 1; channel++) {
        each_channel_perform(id, &perform[channel]);
        gen_next_id(id,&next_id,1,1);
        id=next_id;
    }
    
    /* calculate the best, worst and average of all channels*/
    perform_t best_perform = {0.0, 9999.99, 0.0};
    for (channel = 0; channel < 1; channel++) {
        if (best_perform.best < perform[channel].best) {
            best_perform.best = perform[channel].best;
        }
        if (best_perform.worst > perform[channel].worst) {
            best_perform.worst = perform[channel].worst;
        }
        /* temperatly sum the average  to best_perform.average */
        best_perform.avrg += perform[channel].avrg;
    }
    //best_perform.avrg /= SSD_MAX_CHANNEL;

    /* write to ouput file named log_single_channel_perform */
    FILE *fp = fopen("log_multi_channel_perform_write", "w");
    if (NULL == fp) {
        printf("\tERROR! can not open ouput file\n");
        exit(1);
    }
    
    for (channel = 0; channel < 1; channel++) {
        if (0 == channel) {
            fprintf(fp, "the multi channel perform summary write:\n");
            fprintf(fp, "perform best:    %0f M/s\n", best_perform.best);
            fprintf(fp, "perform worst:   %0f M/s\n", best_perform.worst);
            fprintf(fp, "perform average: %0f M/s\n", best_perform.avrg);
        }
        fprintf(fp, "..................................\n");
        fprintf(fp, "the channel No: %d\n", channel);
        fprintf(fp, "perform in channel best:    %0f M/s\n", perform[channel].best);
        fprintf(fp, "perform in channel worst:   %0f M/s\n", perform[channel].worst);
        fprintf(fp, "perform in channel average: %0f M/s\n", perform[channel].avrg);
    }
    
   	SSD_CASE_INFO("w_p","03","SSD close"); 
	SSD_close(0);
    
    exit(0);
}
    
