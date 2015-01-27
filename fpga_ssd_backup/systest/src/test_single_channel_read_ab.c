#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/api/ssd_api.h"
#include "../include/test_macro.h"
#include "../include/test_tools.h"

int 
main(void)
{
	int  i,ssd_flag,TIMES=2;
	char  *buff, *err_buff,*null_buff;
	BlockId id,next_id,temp_id,id_2m,id_10m;
	uint32_t len,offset;
    	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;

	buff = (char *)malloc(2 * BT_DATA_SIZE*sizeof(char));//20MB
	if (NULL == buff) {
		printf("can not malloc buff\n");
		ret = SSD_TEST_ERROR;
        exit(-1);
	}
	
	for(i = 0; i < 2*BT_DATA_SIZE; i++)
		*(buff+i) = rand();
	
	SSD_CASE_INFO("10","ALL","TEST SSD read abnormal");	
	/* write non ssd device */
	SSD_CASE_INFO("10","01","read,but device ssd is not open");
	id.m_nLow=0;
    	id.m_nHigh=0;
	offset=get_randm_offset(id);
   	len=get_randm_read_len(id,offset);
    	read_process( id,  buff, offset,len, SSD_ERR_NOT_OPENED);


	SSD_CASE_INFO("10","02","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("10","02","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("10","02","OPEN THE SSD DEVICE");
        }

  	/*write 10 times*/
	 SSD_CASE_INFO("10","03","SSD write TIMES");
  	gen_first_id(&id);
   	gen_next_id(id,&next_id,1,1);
	id=next_id;
	id_10m=next_id ;
	len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
	write_process( id_10m,  buff,len,SSD_OK);	
	gen_next_id(id,&next_id,0,1);
	id_2m=next_id;
	id=next_id;
	len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
	write_process( id_2m,  buff,len,SSD_OK);

	/* read id is not exist */
	SSD_CASE_INFO("10","04","read,but id is not exist");
	gen_next_id(id,&next_id,1,1);
	id=next_id;
	offset=get_randm_offset(id);
    	len=get_randm_read_len(id,offset);
    	read_process(id, buff, offset,len,SSD_ID_NO_EXIST);          ///SSD to be changed
    
   	/* write len error */
	SSD_CASE_INFO("10","05","read,but len is not 8k");
	
		
	id=id_2m;
    	len=8*1024+1;
		
	offset=0;
    	read_process( id,  buff, offset,len,SSD_ERR_LEN_PAGE_ALIGN);
        id=id_10m;
        len=8*1024+1;
        offset=0;
        read_process( id,  buff, offset,len,SSD_ERR_LEN_PAGE_ALIGN);

	SSD_CASE_INFO("10","06","read,len is over 2m or 10m");
        id=id_2m;
        len=2*1024*1024+8*1024;
        offset=0;
        read_process( id,  buff, offset,len,SSD_ERR_PARAM_OVERFLOW);
	
		
	id=id_10m;
        len=10*1024*1024+8*1024;
        offset=0;
        read_process( id,  buff, offset,len,SSD_ERR_PARAM_OVERFLOW);
        
	SSD_CASE_INFO("10","07","offset+len is bigger than 2m or 10m");
        id=id_2m;
        len=2*8*1024;
        offset=2*1024*1024-8*1024;
        read_process( id,  buff, offset,len,SSD_ERR_PARAM_OVERFLOW);
	
		
	id=id_10m;
        len=2*8*1024;
        offset=10*1024*1024-8*1024;
        read_process( id,  buff, offset,len,SSD_ERR_PARAM_OVERFLOW);


  	/* write offset error */
	SSD_CASE_INFO("10","08","read,offset is not 8k ");
	
		
	id=id_2m;
    	len=8*1024;
		
	offset=1024*8+1;
    	read_process( id,  buff, offset,len, SSD_ERR_START_PAGE_ALIGN);
		
	id=id_10m;
        len=8*1024;
        offset=1024*8+1;
        read_process( id,  buff, offset,len, SSD_ERR_START_PAGE_ALIGN);
        
	SSD_CASE_INFO("10","09","read,offset is bigger than 2m or 10m");
		
	id=id_2m;
        len=8*1024;
        offset=1024*1024*2+8*1024;
        read_process( id,  buff, offset,len, SSD_ERR_PARAM_OVERFLOW);
		id=id_10m;
        len=8*1024;
        offset=1024*1024*10+8*1024;
        read_process( id,  buff, offset,len, SSD_ERR_PARAM_OVERFLOW);
        
        /* write offset+len error */
	SSD_CASE_INFO("10","10","read,len+offset over 32bit");
        id=id_2m;
		
	len=0xffffffff;
        offset=0xffffffff;
        read_process( id,  buff, offset,len, SSD_ERR_LEN_PAGE_ALIGN);
        id=id_10m;
		
	len=0xffffffff;
        offset=0xffffffff;
        read_process( id,  buff, offset,len, SSD_ERR_LEN_PAGE_ALIGN); 


	SSD_CASE_INFO("10","11","buff is null");
        null_buff=NULL;
		
	id=id_2m;
        offset=0;
        len=8*1024;
        read_process(id, null_buff,offset,len,SSD_ERR_NULL);  
     
         
		/* buff < len  */
	SSD_CASE_INFO("10","12","read,buff is shorter ");
	   
	err_buff = (char *)realloc(buff, (8*1024-10)*sizeof(char));// 
	if(NULL == err_buff){
		printf("can not realloc, buff remains 10MB\n");
		ret = SSD_TEST_ERROR;
	}else{
		buff = err_buff;
		err_buff = NULL;
	}	
	
		offset=0;
    	len=8*1024;
    	read_process(id, buff,offset,len,SSD_OK);// ssd_ok to be changed
    

	SSD_CASE_INFO("10","13","SSD close ");
	SSD_close(0);
  
	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("10","ALL","ssd read abnormal test pass ");
	else
		SSD_CASE_INFO("10","ALL","ssd read abnormal test fail ");

	return 0;
}


