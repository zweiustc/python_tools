#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../include/test_tools.h"
#include "../include/test_macro.h"

 

 
 
 

int main(void)
{
	
  	int i,ssd_flag,TIMES=45;
	char *buff;
  	BlockId id;
  	uint32_t len = 0;
  	uint32_t offset = 0;
  	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
    
  	const int BUFFSIZE = BT_DATA_SIZE;  // 10m£¬ 

	buff = (char *)malloc(BUFFSIZE*sizeof(char));
	
	if (NULL == buff) {
		printf("ERROR! can not malloc buff\n");
		ret = SSD_TEST_ERROR;
	}
	SSD_CASE_INFO("07","ALL","TEST SSD read normal");

	SSD_CASE_INFO("07","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("07","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("07","01","OPEN THE SSD DEVICE");
        }
		 
		  
	SSD_CASE_INFO("07","02","SSD write read TIMES"); 
        gen_order_id(TIMES,1);
        gen_randm_id(TIMES);
        for (i=0;i<TIMES;i++){
                id=store_write[i];
                len =(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
                write_process(id,buff,len,SSD_OK);
        }

  
    	
    	for (i=0;i<TIMES;i++){
       		SSD_CYCLE_INFO("read",i ); 
        	id=store_write_temp[i];
    	//	offset=store_offset_temp[i];//get_randm_offset(id);
        //	len=store_read_len_temp[i];//get_randm_read_len(id,offset);
		len =(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
		offset=0;
    		read_process(id,buff,offset,len,SSD_OK);
    	}
      
	SSD_CASE_INFO("07","03","SSD close device");
	SSD_close(0);

	free(buff);

	if (SSD_TEST_OK == ret) 
		SSD_CASE_INFO("07","ALL","SSD read normal test pass");
	else
		SSD_CASE_INFO("07","ALL","SSD read normal test fail");

	return 0;
}

