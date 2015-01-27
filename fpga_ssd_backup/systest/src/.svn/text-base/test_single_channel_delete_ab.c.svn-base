#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../include/test_macro.h"
#include "../include/test_tools.h"

 

int 
main(void)
{
	int  i,ssd_flag,test_times=0;
	char  *buff;
	BlockId id,next_id,temp_id;
	uint32_t len;
    enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;

	buff = (char *)malloc(2 * BT_DATA_SIZE*sizeof(char));//20MB
	if (NULL == buff) {
		printf("can not malloc buff\n");
		ret = SSD_TEST_ERROR;
        exit(-1);
	}
	
	for(i = 0; i < 2*BT_DATA_SIZE; i++)
		*(buff+i) = rand();

	SSD_CASE_INFO("11","ALL","TEST single delete abnormal ");		
	/* write non ssd device */
	SSD_CASE_INFO("11","01"," SSD delete but the SSD is not open");  
	id.m_nLow=0;
    id.m_nHigh=0;
    delete_process( id, SSD_ERR_NOT_OPENED);


	SSD_CASE_INFO("11","02","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("11","02","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("11","02","OPEN THE SSD DEVICE");
        }

  	/*write 10 times*/
	SSD_CASE_INFO("11","03","SSD write test_times"); 
    	gen_first_id(&id);
    	temp_id=id;
     	for (i=0;i<=test_times;i++){
         	len= (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
         	write_process(id,buff,len,SSD_OK);
         	gen_next_id(id,&next_id,len,1);
         	id=next_id;
       }


	/* delete id is not exist */
	SSD_CASE_INFO("11","04","SSD delete not id is not exist");
	gen_next_id(id,&next_id,len,1);
	id=next_id;
      	delete_process(id,SSD_ID_NO_EXIST);      //to be changed

	
        SSD_CASE_INFO("11","05","SSD DEVICE CLOSE");
        SSD_close(0);

	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("11","ALL","SSD delete abnormal test pass");
	else
		SSD_CASE_INFO("11","ALL","SSD delete abnormal test fail");
	return 0;
}


