#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>

//#include "../include/api/ssd_api.c"
#include "../../include/api/ssd_api.h"
#include "../../include/test_macro.h"
#include "../../include/test_tools.h"

#define MEASURE  10

typedef struct _perform {
    float best;
    float worst;
    float avrg;
} perform_t;    /* the best, worst and averge perorm M/s */

/* const int op_actions = 4;  */  /* operation actions: read, write, erase, mix */

void each_channel_perform(const BlockId id, perform_t *perform)
{
    //char device[16];
    BlockId read_id,write_id,next_id;
    uint32_t len,offset;
    const int BUFFSIZE = BT_DATA_SIZE;
    char *buff_w = (char*)malloc(BUFFSIZE * sizeof(char)); /* 10MB*/
    char *buff_r = (char*) malloc(BUFFSIZE * sizeof(char)); /* 10MB*/
    if ((NULL == buff_r)||(NULL == buff_w)) {
        printf("\tERROR! can not malloc \n");
        exit(3);
    }

     write_id=id;
     read_id=write_id;
     len=BT_DATA_SIZE;
     offset=0;
     
    int i; /* measure 3 times to pick up the best, worst and average */
    int j; /* 50 times operations in every measure */
    struct timeval s_time, e_time;

    for (i = 0; i < 3; i++) {
        float temp_perform = 0.0;
        int vol = BUFFSIZE / (1024 * 1024); /* read blocks */

        for (j = 0; j < MEASURE; j++) {
           len =(write_id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
           write_process(write_id,buff_w,len,SSD_OK);
           gen_next_id(write_id,&next_id,1,44);
	   write_id=next_id;
        }
          
        gettimeofday(&s_time, NULL);
        for (j = 0; j < MEASURE; j++) {
           read_process(read_id,buff_r,offset,len,SSD_OK);
           gen_next_id(read_id,&next_id,1,44);
    	   read_id=next_id;
        }
        gettimeofday(&e_time, NULL);

        double temp = (e_time.tv_sec - s_time.tv_sec)
            + (e_time.tv_usec - s_time.tv_usec) / 1000000.0;
        printf("in loop: temp = %lf\n", temp);
        temp_perform = (MEASURE * vol) / ((e_time.tv_sec - s_time.tv_sec)
            + (e_time.tv_usec - s_time.tv_usec) / 1000000.0);
        printf("in loop : %f\n",temp_perform);

        if (temp_perform < perform->worst)
            perform->worst = temp_perform;
        else if (temp_perform > perform->best)
            perform->best = temp_perform;
        perform->avrg += temp_perform;
    }
    perform->avrg /= 3.0;

  //  ssd_close(fp);
}

int main(void)
{
    srand((unsigned)time(NULL));
    
//    int SSD_MAX_CHANNEL=44;
    perform_t perform[SSD_MAX_CHANNEL];     /* each channel perform data */
    BlockId id,next_id;
    int ssd_flag;
    enum SSD_TEST_RET_TYPE ret;
    /*open the ssd device*/
        
    
	SSD_CASE_INFO("r-p","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("r-p","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("r-p","01","OPEN THE SSD DEVICE");
        }

    int channel;
    for (channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
        perform[channel].best = 0.0;
        perform[channel].worst = 99999999.99;
        perform[channel].avrg = 0.0;
    }

    /* calculate each channel perform */
	SSD_CASE_INFO("r-p","02","test the read performance");
    gen_first_id(&id);
    for (channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
        each_channel_perform(id, &perform[channel]);
        gen_next_id(id,&next_id,1,1);
        id=next_id;
    }
    
    /* calculate the best, worst and average of all channels*/
    perform_t best_perform = {0.0, 99999999.99, 0.0};
    for (channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
        if (best_perform.best < perform[channel].best) {
            best_perform.best = perform[channel].best;
        }
        if (best_perform.worst > perform[channel].worst) {
            best_perform.worst = perform[channel].worst;
        }
        /* temperatly sum the average  to best_perform.average */
        best_perform.avrg += perform[channel].avrg;
    }
    best_perform.avrg /= SSD_MAX_CHANNEL;

    /* write to ouput file named log_single_channel_perform */
    FILE *fp = fopen("log_single_channel_perform_read", "w");
    if (NULL == fp) {
        printf("\tERROR! can not open ouput file\n");
        exit(1);
    }
    
    for (channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
        if (0 == channel) {
            fprintf(fp, "the single channel perform summary read:\n");
            fprintf(fp, "perform best:    %0f M/s\n", best_perform.best);
            fprintf(fp, "perform worst:   %0f M/s\n", best_perform.worst);
            fprintf(fp, "perform average: %0f M/s\n", best_perform.avrg);
        }
        fprintf(fp, "the channel No: %d\n", channel);
        fprintf(fp, "perform in channel best:    %0f M/s\n", perform[channel].best);
        fprintf(fp, "perform in channel worst:   %0f M/s\n", perform[channel].worst);
        fprintf(fp, "perform in channel average: %0f M/s\n", perform[channel].avrg);
    }
   
	SSD_CASE_INFO("r-p","03","close the ssd device");
  	SSD_close(0);  
    exit(0);
}
    
