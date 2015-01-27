
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
 #include "../include/api/ssd_api.h"
 //#include "../include/api/ssd_api.c"
 #include "../include/test_tools.h"
 #include "../include/test_macro.h"





int 
main(void)
{
	int  i,ssd_flag,TIMES;
	char  *buff;
    	
	char  *err_buff,*null_buff;
	BlockId id,next_id;
	uint32_t len;
    	
	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
    
	buff = (char *)malloc(2 * BT_DATA_SIZE * sizeof(char) );//20MB
	if (NULL == buff) {
		printf("can not malloc buff\n");
		ret = SSD_TEST_ERROR;
	}
	
	for(i = 0; i < 2*BT_DATA_SIZE; i++)
		*(buff+i) = rand();
	
	SSD_CASE_INFO("09","ALL","TEST SSD write  abnormal");		
	/* write non ssd device */
	SSD_CASE_INFO("09","01"," write,but SSD is not open");
	
	id.m_nLow=0;
    	id.m_nHigh=0;
	len=2*1024*1024;
    	write_process(id,buff,len,SSD_ERR_NOT_OPENED);



	SSD_CASE_INFO("09","02","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("09","02","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("09","02","OPEN THE SSD DEVICE");
        }
  
 
  	/* write same id */                                     ///????????????????????????????
	SSD_CASE_INFO("09","03","write same id");
	gen_first_id(&id);
	len= (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
    	write_process( id,  buff, len, SSD_OK);
    	write_process( id,  buff, len, SSD_ID_ALREADY_EXIST);//to be changed
    	delete_process(id,SSD_OK);
   
        /* write len is not 2m or 10m */
	SSD_CASE_INFO("09","04","len is not 2m or 10m");
        gen_next_id(id,&next_id,0,1);
        id=next_id;
        len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
        len = len+1;
        write_process( id,  buff, len,SSD_ERR_LEN_PAGE_ALIGN); //to be changed
	

         /* write len and id not the same */

       
	SSD_CASE_INFO("09","05","len and id is not match"); 
        gen_next_id(id,&next_id,0,1);
        id=next_id;
        len = BT_DATA_SIZE;
        write_process( id,  buff, len,SSD_ERR_LEN_PAGE_ALIGN); //SSD_ERR_LEN_BLOCK_ALIGN 
        gen_next_id(id,&next_id,1,1);
        id=next_id;
        len = BT_INDEX_SIZE;
        write_process( id,  buff, len,SSD_ERR_LEN_PAGE_ALIGN); //SSD_ERR_LEN_BLOCK_ALIGN



	SSD_CASE_INFO("09","06","buff is null");
        null_buff=NULL;
        gen_next_id(id,&next_id,1,1);
        len=(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
        write_process( id,  null_buff, len, SSD_ERR_NULL);

	/* buff < len  */
	SSD_CASE_INFO("09","07","buff is shorter");
	err_buff = (char *)realloc(buff, (BT_INDEX_SIZE)*sizeof(char));//2MB
	if(NULL == err_buff){
		printf("can not realloc, buff remains 10MB\n");
		ret = SSD_TEST_ERROR;
         	exit(-1);
	}else{
		buff = err_buff;
		err_buff = NULL;
		}	
    	gen_next_id(id,&next_id,1,1);//gen 10m id
	len=0;
    	write_process( id,  buff, len,  SSD_ERR_LEN_PAGE_ALIGN);//??????
  
  
   	/*the ssd is full*/
	/*
	SSD_CASE_INFO("09","08","write full"); 
    	gen_next_id(id,&next_id,1,1);
    	id=next_id;
    	len=1;
    	TIMES=SSD_MAX_VIRBLOCKS_PER_CHANNEL/BT_DATA_BLOCK_NUM;    
    	for (i=0;i<TIMES;i++){
    		printf("case 09-08: write the %d times \n",i);
    		write_process(id,buff,len,SSD_OK);
    		gen_next_id(id,&next_id,1,44);
    		id=next_id;
    	}

        write_process(id,buff,len,SSD_ERR_NO_INVALID_BLOCKS);//to be changed
	*/

	SSD_CASE_INFO("09","09","SSD close device");
        SSD_close(0); 
	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("09","ALL","TEST SSD write  abnormal test pass");
	else
		SSD_CASE_INFO("09","ALL","TEST SSD write  abnormal test fial");

	return 0;
}

