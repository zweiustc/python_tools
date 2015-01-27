#include <stdio.h>
#include <time.h>
#include "../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
#include "../include/test_tools.h"
#include "../include/test_macro.h"

int main(void)
{
     	int i,j,m,TEST_TIMES=44;
    	int FLUSH_TIMES=20*21+1;
     	enum SSD_TEST_RET_TYPE ret = SSD_TEST_OK;
     	int ssd_flag = SSD_ERROR;
     	BlockId id,next_id;
     	uint32_t len,offset;

     	const int BUFFSIZE = BT_DATA_SIZE;   //10m￡?
     	char*buff = (char *)malloc(BUFFSIZE*sizeof(char));
		const int BUFFTABLE=8*1024;
	//	char*buff_addr=(char *)malloc(BUFFTABLE*sizeof(char));
	//	char*buff_erase=(char *)malloc(BUFFTABLE*sizeof(char));
		char buff_addr[BUFFTABLE];
		char buff_erase[BUFFTABLE];
		char buff_addr_all[44][BUFFTABLE];
		char buff_erase_all[44][BUFFTABLE];
	//	for(i=0;i<44;i++){
	//			buff_addr_all[i]=(char *)malloc(BUFFTABLE*sizeof(char));
	//			buff_erase_all[i]=(char *)malloc(BUFFTABLE*sizeof(char));
	//	}

		
     	if (NULL == buff) {
     	printf("ERROR! can not malloc buff\n");
     	ret = SSD_TEST_ERROR;
     	}
	
	//需要根据具体的SSD的错误种类进行修改
	printf("case 03-01: ssd_flush() but not ssd_open before\n");
	if ((ssd_flag = SSD_flush(0)) != -SSD_ERR_NOT_OPENED) {
		printf("\tERROR!");
		printf("\tcase 03-01, expected: %d, actual: %d\n", SSD_ERR_NOT_OPENED, ssd_flag); 
		ret = SSD_FLUSH_ERROR;
        exit(-1);
	} else {
		printf("ssd_flush SSD_ERR_NOT_SSD returns successfully\n");
	}

 	printf("case 03-02: ssd_open()\n");
	if ((ssd_flag = SSD_open(0)) != SSD_OK) {
		printf("\tERROR!\n");
		printf("\tcase03-02, excepted: %d, actual: %d\n", SSD_OK, ssd_flag);
		ret = SSD_OPEN_ERROR;
        exit(-1);
	} else {
		printf("case 03-02: SSD_OPEN returns successfully\n");
	}

     
	printf("case 03-03: random write test_times times tesing\n");
      
     	id.m_nLow=0;
      	id.m_nHigh=0;
      	for (i=0;i<TEST_TIMES;i++){
        	 printf("case 03-03: write the %d times \n",i);
         	len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
         	write_process(id,buff,len,SSD_OK);
         	gen_next_id(id,&next_id,1,1);
         	id=next_id;
         }
	
		
        printf("case 03-04: ssd_flush\n");
        if ((ssd_flag = SSD_flush(0)) != SSD_OK) {
                printf("\tERROR!\n");
                printf("\tcase 03-04, excepted: %d, actual: %d\n", SSD_OK, ssd_flag);
                ret = SSD_FLUSH_ERROR;
                exit(-1);
        } else {
                printf("case 03-04: SSD_flash returns successfully\n");
		}


		for(i=0;i<44;i++)
		{
				memset(buff_erase_all[i],0,BUFFTABLE);
				memset(buff_addr_all[i],0,BUFFTABLE);
				SSD_read_erasetable(i, buff_erase_all[i]);
				SSD_read_addrtable(i, buff_addr_all[i]);
		}

		
        for(j=0;j<FLUSH_TIMES;j++){



        	printf("case 03-04: ssd_flush\n");
        	if ((ssd_flag = SSD_flush(0)) != SSD_OK) {
                	printf("\tERROR!\n");
                	printf("\tcase 03-04, excepted: %d, actual: %d\n", SSD_OK, ssd_flag);
                	ret = SSD_FLUSH_ERROR;
                	exit(-1);
        	} else {
                	printf("case 03-04: SSD_flash returns successfully\n");
        	}

		for(i=0;i<44;i++)
				       
		{
						
			memset(buff_erase,0,BUFFTABLE);
		             
			SSD_read_erasetable(i, buff_erase);
			for(m=0;m<BUFFTABLE;m++){
				
				//printf("the data is %llx",buff_erase[m]);
				//printf("the data is %llx\n",buff_erase_all[i][m]);
				if(buff_erase[m]!=buff_erase_all[i][m]){
					SSD_ERR("c","0","SSD_table_compare");
					}

				}

/*
			if (compare_m(buff_erase, buff_erase_all[i], BUFFTABLE) != 0 ) {
					        
					SSD_RESULT("channel","erasetable", "",i,i,0);
					SSD_ERR("c","0","SSD_table_compare");
									     
			}
*/			
			memset(buff_addr,0,BUFFTABLE); 
												                
			SSD_read_addrtable(i, buff_addr);
/*			for(m=0;m<BUFFTABLE;m++){

                                //printf("the data is %llx",buff_addr[m]);
                                //printf("the data is %llx\n",buff_addr_all[i][m]);
                                if(buff_addr[m]!=buff_addr_all[i][m]){
                                        SSD_ERR("c","0","SSD_table_compare");
                                        }

                                }
/*		   if (compare_m(buff_erase, buff_addr_all[i], BUFFTABLE) != 0 ) {
			   		SSD_RESULT("channel","erasetable", "",i,i,0);
				 	SSD_ERR("c","0","SSD_table_compare");	 
		   }	
			
*/																        
		}

        

        

        
		}

		free(buff);
		//free(buff_addr);
		//free(buff_erase);
		//free(buff_addr_all);
		//free(buff_erase_all);


		SSD_close(0);

        if (ret == SSD_TEST_OK)
                printf("test case 03:  ssd flush device test PASS!!!\n");
        else
                printf("test case 03:  ssd flush device test FAIL!!!\n");
	
	return 0;
}

