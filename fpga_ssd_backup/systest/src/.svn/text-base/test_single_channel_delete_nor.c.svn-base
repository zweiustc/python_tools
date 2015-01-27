#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../include/test_tools.h"
#include "../include/test_macro.h"


int main(void)
{
	
	int i,ssd_flag,TIMES;
  	char *buff;
  	BlockId id ;
  	uint32_t len = 0;
  	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
    
  	const int BUFFSIZE = BT_DATA_SIZE;  // 10m
  	buff = (char *)malloc(BUFFSIZE*sizeof(char));
	
	if (NULL == buff) {
		printf("ERROR! can not malloc buff\n");
		ret = SSD_TEST_ERROR;
        exit(-1);
	}

  
	SSD_CASE_INFO("08","ALL","TEST single delete normal ");
	
	SSD_CASE_INFO("08","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("08","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("08","01","OPEN THE SSD DEVICE");
        }
		   
        SSD_CASE_INFO("08","02","SSD write delete TIMES");
	
		TIMES=45;
    	gen_order_id(TIMES,1);
        gen_randm_id(TIMES);
    	for (i=0;i<TIMES;i++){
    		id=store_write[i];                
			len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
    		write_process(id,buff,len,SSD_OK);
    	}
    	
    
    	for (i=0;i<TIMES;i++){
        	SSD_CYCLE_INFO("delete",i );
		
			id=store_write_temp[i];
    		delete_process(id,SSD_OK);
        
    	}
    	
    	  
     	SSD_CASE_INFO("08","03","SSD DEVICE CLOSE"); 
	
		SSD_close(0);

	free(buff);

	if (SSD_TEST_OK == ret) 
		SSD_CASE_INFO("08","ALL","SSD delete normal test pass");
	else
		SSD_CASE_INFO("08","ALL","SSD delete normal test fail");

	return 0;
}

