#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../include/test_tools.h"
#include "../include/test_macro.h"


 


int main(void)
{
	
  	int i,TIMES=8000;
  	int DEL_TIMES=10;
  	char *buff;
  	BlockId id,next_id,temp_id;
  	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
    
	uint32_t len;  
  	const int BUFFSIZE = BT_DATA_SIZE;   //10m£¬

  	buff = (char *)malloc(BUFFSIZE*sizeof(char));
	
	if (NULL == buff) {
		printf("ERROR! can not malloc buff\n");
		ret = SSD_TEST_ERROR;
	}

  	printf("case 06-01: ssd_open()\n");
	if ( SSD_open(0)!= SSD_OK) {
		printf("\tERROR!\n");
		printf("\tcase06-01, excepted: %d", SSD_OK);
		ret = SSD_OPEN_ERROR;
        exit(-1);
	} else {
		printf("case 06-01: SSD_OPEN returns successfully\n");
	}

	printf("case 06-02: random write TIMES make the channel full\n");
    	gen_first_id(&id);
    	for (i=0;i<TIMES;i++){
    		printf("case 06-03: write the %d times \n",i);
    		write_process(id,buff,1,SSD_OK);
    		gen_next_id(id,&next_id,0,44);
        	store_write[i]=id;
    		id=next_id;
    	}
        
        printf("case 06-03: delete one 2m BLOCK and write one 2m block \n");
        gen_randm_id(TIMES);
        for (i=0;i<1;i++){
          printf("case 06-03: delete the %d times \n",i);
           id=store_write_temp[i];
           delete_process(id,SSD_OK);
            
        }       

         gen_next_id(store_write[TIMES-1],&next_id,0,44);
         id =next_id;
         temp_id=id;
         len =  BT_INDEX_SIZE ;
         write_process(id,buff,len,SSD_OK);
        
 
    	printf("case 06-04: delete DEL_TIMES 2m BLOCK ,write 10m block\n");
      	gen_randm_id(TIMES);
      	for (i=2;i<12;i++){
          printf("case 06-03: delete the %d times \n",i);
           id=store_write_temp[i];
           delete_process(id,SSD_OK);
           
        }
  
        id = temp_id;
        gen_next_id(id,&next_id,1,44);
        id =next_id;
        len =  BT_DATA_SIZE ;
        write_process(id,buff,len,SSD_OK);
        
	gen_next_id(id,&next_id,1,44);
        id =next_id;
        temp_id=id;
        len =  BT_DATA_SIZE ;
        write_process(id,buff,len,SSD_OK);
        
        
        
        printf("case 06-05: delete DEL_TIMES 10m BLOCK ,write 10m block\n");
       
        id=temp_id;
        delete_process(id,SSD_OK);
        gen_next_id(id,&next_id,1,44);
        id =next_id;
        temp_id=id;
        len =  BT_DATA_SIZE ;
        write_process(id,buff,len,SSD_OK);

        
        printf("case 06-06: delete DEL_TIMES 10m BLOCK ,write 2m block\n");

        id=temp_id;
        delete_process(id,SSD_OK);
       
        for (i=0;i<5;i++){
           gen_next_id(id,&next_id,0,44);
           id=next_id;
           len=BT_INDEX_SIZE;
           write_process(id,buff,len,SSD_OK);

        }




     	printf("case 06-06: ssd_close\n");	
/*	if ( SSD_close() != SSD_OK) {
	  	printf("\tERROR!\n");
	  	ret = SSD_CLOSE_ERROR;
        exit(-1);
	} else {
	  	printf("case 06-05: SSD_CLOSE returns successfully\n");
	}
*/      SSD_close(0);

	free(buff);

	if (SSD_TEST_OK == ret) 
		printf("test cases 06 write operation(normal) PASS!!!\n");
	else
		printf("test cases 06 wirte operation(normal) FAIL!!!\n");

	return 0;
}

