#include <stdio.h>
#include <time.h>
#include "../include/api/ssd_api.h"
#include "../include/test_tools.h"
#include "../include/test_macro.h"

int main(void)
{
    int i,TEST_TIMES=44;
    enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
    int ssd_flag=SSD_ERROR;
    BlockId id,next_id,temp_id;
    uint32_t len,offset;
   
	SSD_CASE_INFO("02-01","ALL","TEST close SSD normal "); 
    
	const int BUFFSIZE = BT_DATA_SIZE;  // 10m£¬ 
	char * buff = (char *)malloc(BUFFSIZE*sizeof(char));
	if (NULL == buff) {
	SSD_ERR("02-01","01","can't alloct buff");
	ret = SSD_TEST_ERROR;
	}
    
	SSD_CASE_INFO("02-01","02","OPEN THE SSD DEVICE");
	if ((ssd_flag = SSD_open(0)) != SSD_OK) {
	SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
	SSD_ERR("02-01","02","OPEN THE SSD DEVICE");
	ret = SSD_OPEN_ERROR;
	} else {
	SSD_SUCCESS("02-01","02","OPEN THE SSD DEVICE");
	}

	SSD_CASE_INFO("02-01","03","RANDOM WRITE TEST_TIMES");
	gen_first_id(&id);   
	temp_id=id;
	len=(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;

      for (i=0;i<TEST_TIMES;i++){
	SSD_CYCLE_INFO("write",i );
	write_process(id,buff,len,SSD_OK);
	gen_next_id(id,&next_id,1,1);
	id=next_id;
	len=(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
     }
	
	SSD_CASE_INFO("02-01","04","SSD DEVICE CLOSE");
	SSD_close(0); 
  
      
	SSD_CASE_INFO("02-01","05","OPEN THE SSD DEVICE");
      
	if ((ssd_flag = SSD_open(0)) != SSD_OK) {
	
		SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("02-01","05","OPEN THE SSD DEVICE");
		ret = SSD_OPEN_ERROR;
	} else {
	
		SSD_SUCCESS("02-01","05","OPEN THE SSD DEVICE");
	}

    SSD_CASE_INFO("02-01","06","RANDOM READ TEST_TIMES");
	id=temp_id;
	for (i=0;i<TEST_TIMES;i++){
	SSD_CYCLE_INFO("read",i );
	offset=get_randm_offset(id);
	len=get_randm_read_len(id,offset);
	read_process(id,buff,offset,len,SSD_OK);
	gen_next_id(id,&next_id,1,1);
	id=next_id;
        }
    
	SSD_CASE_INFO("02-01","07","RANDOM DELETE TEST_TIMES"); 
	id=temp_id;
	for (i=0;i<TEST_TIMES;i++){
	SSD_CYCLE_INFO("delete",i );
	delete_process(id,SSD_OK);
	gen_next_id(id,&next_id,1,1);
	id=next_id;
        }  
    
	SSD_CASE_INFO("02-01","08","SSD DEVICE CLOSE"); 
	SSD_close(0);
	
	SSD_CASE_INFO("02-01","09","OPEN THE SSD DEVICE");
	if ((ssd_flag = SSD_open(0)) != SSD_OK) {
	
		SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("02-01","09","OPEN THE SSD DEVICE");	
		ret = SSD_OPEN_ERROR;
	} else {
	
		SSD_SUCCESS("02-01","09","OPEN THE SSD DEVICE");
	}
    
	SSD_CASE_INFO("02-01","10","RANDOM READ TEST_TIMES");    
	id=temp_id;
        for (i=0;i<TEST_TIMES;i++){
            SSD_CYCLE_INFO("read",i );
            offset=get_randm_offset(id);
            len=get_randm_read_len(id,offset);
            read_process(next_id,buff,offset,len,SSD_ID_NO_EXIST);  
            gen_next_id(id,&next_id,1,1);
            id=next_id;
         }  
	
	SSD_CASE_INFO("02-01","11","SSD DEVICE CLOSE");
        SSD_close(0);   
   
 
	if (ret == SSD_TEST_OK)
	SSD_CASE_INFO("02-01","ALL","close SSD normal test pass");
	else
	SSD_CASE_INFO("02-01","ALL","close SSD normal test fail");
	
	return 0;
}

