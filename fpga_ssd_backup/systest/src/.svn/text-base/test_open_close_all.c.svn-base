#include <stdio.h>
#include <fcntl.h>
#include <pthread.h>
#include "../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../include/test_tools.h"
#include "../include/test_macro.h"

int
main(void)
{
        int i,j,times=20*21+1;
        //enum SSD_ERROR_TYPE ssd_flag = SSD_ERROR;
        int  NUM,SSD_THREAD=100;
        pthread_t tidp[SSD_THREAD];
        int TIMES;
        void *tret;
        int  ssd_flag =SSD_ERROR;
        enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
        SSD_CASE_INFO("18","ALL","TEST open and close ");


        SSD_CASE_INFO("18","01","TEST open and close data ");
        for(i=0;i<times;i++){
                 SSD_CYCLE_INFO("open_close",i );
                if ((ssd_flag = SSD_open(0)) != SSD_OK) {
                        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
                        SSD_ERR("18","01","OPEN THE SSD DEVICE");
                        ret = SSD_OPEN_ERROR;
                        } else {
                        SSD_SUCCESS("18","01","OPEN THE SSD DEVICE");
                        }

                TIMES=50*2;
                gen_order_id(TIMES,1);  //gen the id 44*20
                gen_randm_id(TIMES);

                SSD_CASE_INFO("18","01","each thread for write read delete");
                SSD_THREAD=50;
                struct thread_coeff para[50];
                para[0].start_id=store_write[0];
                para[0].step=1;
                para[0].start_num=0;
                para[0].id_num=5;
                for(j=1;j<50;j++)
                {
                para[j].start_id=store_write[0];
                para[j].step=1;
                para[j].start_num= para[j-1].start_num+5;
                para[j].id_num=5;
                }



                for (j = 0; j < SSD_THREAD; j++) {
                        if (pthread_create(&tidp[j], NULL, thr_fn_write_read_delete, &para[j]) != 0) {
                                ret = SSD_TEST_ERROR;
                                SSD_CYCLE_INFO("multi",j );
                                SSD_ERR("","","cant not creat thread multi");
                        }


                  }



                for (j = 0; j < SSD_THREAD; j++) {
                        if (pthread_join(tidp[j], &tret) != 0) {
                                ret = SSD_TEST_ERROR;
                                SSD_CYCLE_INFO("join",j );
                                SSD_ERR("","","cant not join thread d");
                        }
                }


                SSD_close(0);
        }



        if (SSD_TEST_OK == ret)
                SSD_CASE_INFO("18","ALL","open and close test pass");
        else

                SSD_CASE_INFO("18","ALL","open and close test fail");

        return 0;
}
           
