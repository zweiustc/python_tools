#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../include/test_tools.h"
#include "../include/test_macro.h"


enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;

inline static void write_read_delete_process(const struct BlockId id, const uint32_t write_len,\
		const uint32_t read_len,const uint32_t offset, char *buff_w, char *buff_r)
{
	//enum SSD_ERROR_TYPE ssd_close_flag = SSD_ERROR;

	write_process(id,buff_w,write_len,SSD_OK);

 	read_process(id,buff_r,offset,read_len,SSD_OK);
 
     	delete_process(id,SSD_OK);
 

	if (compare_m(buff_w+offset, buff_r, read_len) != 0 ) {
		SSD_RESULT("id.m_nLow","read_len", "offset",id.m_nLow,read_len,offset);
		SSD_ERR("c","0","SSD_data_compare");
	 }
}

 

int main(void)
{
	int  i,ssd_flag;
	int TIMES;
 
	char *buff; 
	char *old_buff;
	uint32_t write_len=0,read_len,offset;
	BlockId id;
    	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
	const unsigned int BUFFSIZE = BT_DATA_SIZE;//10M
	
	SSD_CASE_INFO("15","ALL","TEST write read compare data ");

	buff =(char*) malloc(BUFFSIZE*sizeof(char));
	old_buff = (char*)malloc(BUFFSIZE*sizeof(char));
	if (NULL == buff || NULL == old_buff) {
		printf("ERROR! can not malloc buff\n");
		ret = SSD_TEST_ERROR;
        	exit(-1);
	}

    	memset(buff, 0, BUFFSIZE);
    	memset(old_buff, 1, BUFFSIZE);
    	srand((unsigned)time(NULL));
    
	SSD_CASE_INFO("15","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("15","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("15","01","OPEN THE SSD DEVICE");
        }
	

	SSD_CASE_INFO("15","02","gen ramdom TIMES of id");   
    	TIMES=1000;
    	gen_order_id(TIMES,1);  
    	gen_randm_id(TIMES);
    
    	
	SSD_CASE_INFO("15","03","begin RANDOM WRITE read compare TEST_TIMES");
 
    	for(i=0;i<TIMES;i++){
 		
			SSD_CYCLE_INFO("write_read_delete_compare",i );
    		id=store_write_temp[i];
                
			write_len=(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
    		offset=get_randm_offset(id);
 		
			read_len=get_randm_read_len(id,offset);
    		write_read_delete_process( id, write_len, read_len, offset,buff,old_buff);
 
		
	}		
		
	free(buff);
	free(old_buff);

	SSD_CASE_INFO("15","04","SSD DEVICE CLOSE");
        SSD_close(0);

	if(SSD_TEST_OK == ret)
	SSD_CASE_INFO("15","ALL","write read compare data test pass");
	else
	SSD_CASE_INFO("15","ALL","write read compare data test fail");

	return 0;
}
