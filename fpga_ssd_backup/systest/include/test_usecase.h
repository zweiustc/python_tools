#ifndef __SSD_TEST_TOOLS_H__
#define __SSD_TEST_TOOLS_H_

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"
 
#include "../include/test_macro.h"

/*******  function declaration  **********/
uint32_t random_m(uint32_t );
int compare_m(const void *old_buff, const void *buff, uint64_t size);

void *thr_fn_util(void * arg);


void *thr_fn_read(void * arg);
void *thr_fn_read_ab(void * arg);
void *thr_fn_write(void * arg);
void *thr_fn_write_ab(void * arg);
void *thr_fn_delete(void *arg);
void *thr_fn_delete_ab(void *arg);

void gen_order_id(const int num,const uint64_t step);
void gen_randm_id(const int num);
void gen_next_id(const struct BlockId id,   struct BlockId *next_id,int write_len,uint64_t n);
static inline void gen_first_id( struct BlockId *id);
uint32_t get_randm_offset(const struct BlockId id);
uint32_t get_randm_read_len(const struct BlockId id,uint32_t offset);

int delete_all_id(const char *file ,const struct stat *sb,int flag);

int print_all_id( const char *file , const struct stat *sb ,int flag);
int delete_all_add_id(const char *file , const struct stat *sb,int flag);
int print_all_add_id(const char *file ,  const struct stat *sb ,int flag);

void write_process(const struct BlockId id,  char * const buffer,const uint32_t len,const int status);
void delete_process(const struct BlockId id,const int status);
void read_process(const struct BlockId id,  char * const buffer,const uint32_t offset,const uint32_t len,const int status);

void SSD_ERR( char * msg,...);
void SSD_SUCCESS( char * msg,...);
void SSD_CYCLE_INFO(const char * a1, int b1 );
void SSD_CASE_INFO( char *msg,...);
void SSD_RESULT(const char * a1, const char * a2,const char * a3,long b1,long b2,long b3 );

struct BlockId store_write[1600];

struct BlockId store_write_temp[8000];
uint32_t store_offset[8000];
uint32_t store_offset_temp[8000];
uint32_t store_read_len[8000];
uint32_t store_read_len_temp[8000];


struct thread_coeff
{
    BlockId start_id;
    int start_num;
    int step;
    int id_num;
    int write_num_1;
    int write_num_2;
    int read_num_1;
    int read_num_2;
    int delete_num_1;
    int delete_num_2;
    int flush_num;
    int util_num;
};


void gen_order_id(const int num,const uint64_t step) //gen the 10m block
{
    BlockId id,next_id;
    int i,len;
    gen_first_id(&id);
    for(i=0;i<num;i++)
    {
        store_write[i]=id;
        store_write_temp[i]=id;
		//=========
		len =(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
		store_offset[i]=get_randm_offset(id);
		store_offset_temp[i]=store_offset[i];
		store_read_len[i]=get_randm_read_len(id,store_offset[i]);
		store_read_len_temp[i]=store_read_len[i];
		//===============
		
        len=1;      
        gen_next_id(id,&next_id,len,step);
        id=next_id;
    }
    
}



void gen_randm_id(const int num,BlockId * buf,BlockId * buf_temp)
{
  int i,j;  
  for(i=0;i<num;i++){
      buf[i].m_nLow=0;
  }
 
  for(i=0;i<num;i++){
      	j=rand()%num;
      	while(buf[j].m_nLow != 0){
          j=rand()%num;
      	}
      	buf_temp[j]=buf[i];
	  
  }

}


void gen_new_randm_id(const int num,const int total_num,BlockId * buf,BlockId * buf_temp)
{
  int i,j;
  for(i=0;i<total_num-num;i++){
      buf_temp[i]=buf_temp[i+num];
  }
  for(i=0;i<total_num-num;i++){
	  buf[i]=buf_temp[i];
  }


}


/****** function definations  ***********/
/* random function for giant number range */


uint32_t random_m(uint32_t max)
{
    /* double can not remove */
    return (uint32_t)((double)max * rand() / (RAND_MAX + 1.0));
}



int compare_m(const void *old_buff, const void *buff, uint64_t size)
{
    int ret;
    
    ret = memcmp(old_buff, buff, (size_t)size);
    if (0 == ret)
        return 0;
    else
        return -1;
        exit(-1);
}



 void write_process(const struct BlockId id,  char * const buffer,\
                       const uint32_t len,const int status)
{
    
	int flag;//to be changed
    //uint32_t len_new = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
	if ((flag = SSD_write(0,id, buffer,len,0)) != -status) {
	SSD_RESULT("excepted","actual", "else",status,-flag,0 );
	SSD_RESULT("id.m_nLow","id.m_nHigh", "len",id.m_nLow,id.m_nHigh,len );
	SSD_ERR("w","0","SSD_write");
	} else {
	SSD_RESULT("id.m_nLow","id.m_nHigh", "len",id.m_nLow,id.m_nHigh,len );
	SSD_SUCCESS("w","0","SSD_write");
	}

}


void delete_process(const struct BlockId id,const int status)
{
	int flag;
	if ((flag = SSD_delete(0,id)) != -status) {
	SSD_RESULT("excepted","actual", "else",status,-flag,0 );
        SSD_RESULT("id.m_nLow","id.m_nHigh", "else",id.m_nLow,id.m_nHigh,0 );
        SSD_ERR("d","0","SSD_delete");
         } else {
		SSD_RESULT("id.m_nLow","id.m_nHigh", "else",id.m_nLow,id.m_nHigh,1);
        SSD_SUCCESS("d","0","SSD_delete");
         }



}



void read_process(const struct BlockId id,  char * const buffer,\
                 const uint32_t offset,const uint32_t len,const int status)
{


	int flag;
	if ((flag = SSD_read(0,id, buffer, len,offset)) != -status) {
		SSD_RESULT("excepted","actual", "else",status,-flag,0 );
        SSD_RESULT("id.m_nLow","len", "offset",id.m_nLow,len,offset);
		SSD_ERR("r","0","SSD_read");
	} else {
		SSD_RESULT("id.m_nLow","len", "offset",id.m_nLow,len,offset);
		SSD_SUCCESS("r","0","SSD_read");    
	}

}






//=====================================util
 void *
thr_fn_util(void * arg)
{

	
    uint64_t util_flag;
    printf(" thread ssd_util()\n");
    util_flag= SSD_util(0);
    printf(" SSD_UTIL successful, returns %ld \n",util_flag);
    return (void *)0;                                                                                                  
}

 void *
thr_fn_flush(void * arg)
{

        
    uint32_t flag;
    printf(" thread ssd_util()\n");
    flag= SSD_flush(0);
    printf(" SSD_flush successful");
    return (void *)0;
}


//===================write-read-delete--


 void *
thr_fn_compare_divide(void* arg)
{


        int i,j,m,test_time;
        struct thread_coeff * para ;
        para=( struct thread_coeff * )arg;
        BlockId id,next_id,first_id;
        uint32_t len;
        uint32_t offset;

        const int BUFFSIZE=BT_DATA_SIZE; //100M
        char * buff_w;
        char * buff_r;
        buff_r = (char *)malloc(BUFFSIZE*sizeof(char));//
        buff_w = (char *)malloc(BUFFSIZE*sizeof(char));//
        if ((NULL == buff_r)||(NULL ==buff_w)) {
                printf("can not malloc buff in read_ok\n");
                        exit(-1);
        }



        memset(buff_r, 0, BUFFSIZE);
        m=para->step;
        id=para->start_id;
	first_id =id;
	test_time = para->id_num;
// thread write
        for (i=0;i<test_time;i++){


		memset(buff_w, i, BUFFSIZE);
                gen_next_id(id,&next_id,1,m);
                id=next_id;
                len= BT_DATA_SIZE;
                write_process(id,buff_w,len,SSD_OK);

	}
//thread read 
	id=first_id;
       for (i=0;i<test_time;i++){


                gen_next_id(id,&next_id,1,m);
                id=next_id;
                memset(buff_r, 0, BUFFSIZE);
		memset(buff_w, i, BUFFSIZE);
                offset=0;
                len=BT_DATA_SIZE;
                read_process(id,buff_r,offset,len,SSD_OK);


                if (compare_m(buff_w+offset, buff_r, len) != 0 ) {

                        SSD_RESULT("id.m_nLow","read_len", "offset",id.m_nLow,len,offset);

                        SSD_ERR("c","0","SSD_data_compare");

                        }
                else
                        {
                        SSD_SUCCESS("w","0","SSD_data_compare");
                        }

        }	
//thread delete
	id=first_id;
	for (i=0;i<test_time;i++){


                gen_next_id(id,&next_id,1,m);
                id=next_id;
                delete_process(id,SSD_OK);

        }

        free(buff_w);

        free(buff_r);

        return (void *)0;
}

 void *
thr_fn_operation(void* arg)
{


        int i,j,m;
        struct thread_coeff * para ;
        para=( struct thread_coeff * )arg;
        BlockId id,next_id,write_id;

        uint32_t len;
        uint32_t offset;
		uint32_t ssd_flag;
		uint64_t util_flag,total_num=0;
		uint32_t write_num,read_num,delete_num;
		uint64_t STORESIZE;	
        const int BUFFSIZE=BT_DATA_SIZE; //100M
        char * buff_w;
		char * buff_r;
        buff_r = (char *)malloc(BUFFSIZE*sizeof(char));//
		buff_w = (char *)malloc(BUFFSIZE*sizeof(char));
        if ((NULL == buff_r)||(NULL ==buff_w)) {
                printf("can not malloc buff in read_ok\n");
                        exit(-1);
        }

		uint32_t step,id_num,write_num_1,write_num_2,read_num_1,read_num_2;
		uint32_t delete_num_1,delete_num_2,flush_num,util_num;
		id     =para->start_id;
		step   =para->step;
		id_num =para->id_num;

		write_num_1  =para ->write_num_1;
		write_num_2  =para ->write_num_2;
		read_num_1   =para ->read_num_1;
		read_num_2   =para ->read_num_2;
		delete_num_1 =para ->delete_num_1;
		delete_num_2 =para ->delete_num_2;
		flush_num  =para ->flush_num;
		util_num   =para ->util_num;
	
		STORESIZE = id_num*write_num_2;
		BlockId * write_store;
		BlockId * write_store_temp;
		write_store =(BlockId *)malloc(STORESIZE*sizeof(BlockId));
		write_store_temp =(BlockId *)malloc(STORESIZE*sizeof(BlockId));
   		if(NULL == write_store || NULL == write_store_temp){
			printf("can not malloc buff in write_store\n");
			exit(-1);
		}	
	
		for (i=0;i<id_num;i++){
//write operation
                write_num=write_num_1 + rand()%(write_num_2-write_num_1);
                for(j=0;j<write_num;j++){
			
						if(j==0)
						write_id=id;

                        write_process(write_id,buff_w,BT_DATA_SIZE,SSD_OK);
                        write_store[total_num+j]=write_id;
						//flush
						if(write_id.m_nLow%flush_num == 0){
								if ((ssd_flag = SSD_flush(0)) != SSD_OK) {
                        			printf("\tERROR!\n");
                        			printf("\texcepted: %d, actual: %d\n", SSD_OK, ssd_flag);
                				} else {
                        			printf(" SSD_flash returns successfully\n");
                				}
						}
						//util
						if(write_id.m_nLow%util_num == 0){
								if ((util_flag = SSD_util(0)) != 0) {
                        			SSD_RESULT("excepted","actual", "else",0,ssd_flag,0 );
                				} else {
                        			SSD_SUCCESS("","","SSD_util");
                				}
						}
							
                        gen_next_id(write_id,&next_id,1,step);
                        write_id=next_id;

                }
				total_num=total_num + write_num;
				//=================================================
				gen_randm_id(total_num,write_store,write_store_temp);

// read operation
				//==================================================				
                read_num=read_num_1 + rand()%(read_num_2-read_num_1);
              	if(read_num >total_num)
					read_num = total_num;
 				//=================================================
                for(j=0;j<read_num;j++){
                        memset(buff_r, 0, BUFFSIZE);
                        offset=0;
                        len= BT_DATA_SIZE;//get_randm_read_len(id,offset);
                        read_process(write_store_temp[j],buff_r,offset,len,SSD_OK);

                                
						if (compare_m(buff_w+offset, buff_r, len) != 0 ) {
                                        SSD_RESULT("id.m_nLow","read_len", "offset",write_store_temp[j].m_nLow,len,offset);
                                        SSD_ERR("c","0","SSD_data_compare");
                                }
                        else{
							SSD_SUCCESS("w","0","SSD_data_compare");
							
						}
				}

// delete operation	
				delete_num=delete_num_1 + rand()%(delete_num_2-delete_num_1);
				if(delete_num >total_num)
                    delete_num = total_num;

				for(j=0;j<delete_num;j++){
                            delete_process(write_store_temp[j],SSD_OK);
                        }
				gen_new_randm_id(delete_num,total_num,write_store,write_store_temp);
				total_num = total_num - delete_num;

		}


        free(buff_w);
        free(buff_r);
		free(write_store);
		free(write_store_temp);

        return (void *)0;
}





//===============================================gen the different BlockId============== 


void gen_next_id(const struct BlockId id,  struct BlockId *next_id,int write_len,uint64_t n)
{
     if(write_len==0)//2m
     {
	 next_id->m_nHigh=id.m_nHigh & 0xFFFFFFFFFFFFFFF8;
     }
     else//10m
     {
     next_id->m_nHigh=id.m_nHigh | 0x07;
     }
     
	 next_id->m_nLow=id.m_nLow + n;
 
}





static inline void gen_first_id( struct BlockId *id)
{
	
	/*
		TableId+UnitId+TabId+MediaType+BlockType+BlockSerialType
    	30       22     8     1           1            2
     m_Low:64Î»£¬´Ó0µÝÔö
	
	*/
	
	int write_len;
    	write_len=1;
    
	if(write_len==0){
    		id->m_nHigh=0; 
    	}else{
        	id->m_nHigh=0x07;
    	}

	srand((int)time(0));	
	id->m_nLow=random_m((2^64)-44*1600-44*150)+44*1600;
	
	
}

 uint32_t get_randm_offset(const struct BlockId id)
{
	uint32_t write_len,offset;
    	
	write_len=(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
	offset=random_m(write_len)/(8*1024)*(8*1024);
	return offset;
	
	
	
}
	
		
 uint32_t get_randm_read_len(const struct BlockId id,uint32_t offset) //1024*102
{
	
		uint32_t i,read_len,write_len;
    	int temp;
    	write_len=(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
	
    	temp=(write_len-offset)/(8*1024);
	
		i=random_m(temp);
		if(i==0)
		{
			i=1;	
		}
		return read_len=i*8*1024;
	
	
}



int delete_all_id(const  char *file , const struct stat *sb , int flag)

{   
  	if(FTW_F == flag)
        {
	char  name1[100];
        strcpy(name1, file);
        char *name = basename(name1);
	//printf("name is %llx",*name);
        BlockId id;
        char low[16];
        char high[16];
	uint64_t high_hex;
	uint64_t low_hex;

	strncpy(high,name,8);
	high_hex=strtol(high,NULL,16);
	strncpy(high,name+8,8);
	high_hex=(high_hex<<32)+ strtol(high,NULL,16);
	id.m_nHigh=high_hex;
	printf(" HIGH 64bits of the id:%lx",id.m_nHigh);

	strncpy(low,name+16,8);
        low_hex=strtol(low,NULL,16);
        strncpy(low,name+24,8);
        low_hex=(low_hex<<32)+ strtol(low,NULL,16);
        id.m_nLow=low_hex;
        printf(" Low 64bits of the id:%lx\n",id.m_nLow);
	
	delete_process(id,SSD_OK);
	
	
	}
   
    return 0;        

}


				

int print_all_id(const char *file ,const  struct stat *sb ,int flag)

{   
	
        if(FTW_F == flag)
        {
	
				
		
	char name1[16];
	strcpy(name1, file);
        char * name =basename(name1);
        BlockId id;
        char low[16];
        char high[16];
        uint64_t high_hex;
        uint64_t low_hex;

        strncpy(high,name,8);
        high_hex=strtol(high,NULL,16);
        strncpy(high,name+8,8);
        high_hex=(high_hex<<32)+ strtol(high,NULL,16);
        id.m_nHigh=high_hex;
        printf(" HIGH 64bits of the id:%lx",id.m_nHigh);

        strncpy(low,name+16,8);
        low_hex=strtol(low,NULL,16);
        strncpy(low,name+24,8);
        low_hex=(low_hex<<32)+ strtol(low,NULL,16);
        id.m_nLow=low_hex;
        printf(" Low 64bits of the id:%lx\n",id.m_nLow);
        
        }

	return 0;
} 



int print(const char *file ,const struct stat *sb ,int flag)

{

        if(FTW_F == flag)
        {
	char name1[16];
        strcpy(name1, file);
//        char * name =basename(name1);

        printf(" Low 64bits of the id");
        printf(" Low 64bits of the id");
        

        }

        return 0;
}

void SSD_ERR(  char * msg,...)
{
	va_list argp;
        char *para=msg;
	va_start( argp, msg );
	printf("ERROR!!!");
	printf("SSD_CASE [%-8s] ",para);
	para = va_arg( argp, char *);
	printf("STEP [%-3s]:",para); 
	para = va_arg( argp, char *);
	printf("%-s\n",para);
	va_end( argp );     
	//exit(-1);                              
                                        /* ?argp??NULL */    
        //return 0;     
	
}

void SSD_RESULT( const char * a1, const  char * a2, const char * a3,long b1,long b2,long b3 )
{
	 printf("the %-s is:%llx;the %-s is:%llx;the %-s is:%llx\n",a1,b1,a2,b2,a3,b3);
                                        /* ?argp??NULL */
        //return 0;

}
void SSD_CYCLE_INFO(const char * a1, int b1 )

{
	 printf("THE %-8s CYCLE is THE %lx TIMES\n",a1,b1);

}
void SSD_SUCCESS(char * msg,...)
{
        va_list argp;
        char *para=msg;
        va_start( argp, msg );

        printf("SUCCSSFULL!!!\n",para);
        printf("SSD_CASE [%-8s] ",para);
        para = va_arg( argp, char *);
	
		printf("STEP [%-3s]:",para);
        para = va_arg( argp, char *);
	
		printf("%-s\n",para);
        va_end( argp );     
                                        /* ?argp??NULL */    
        //return 0;     
        
}

void SSD_CASE_INFO(  char *msg,...) 
{
	va_list argp;
        char *para=msg;
        va_start( argp, msg );
        printf("SSD_CASE [%-8s]",para);
        para = va_arg( argp, char *);
        printf("STEP [%-3s]:",para); 
        para = va_arg( argp, char *);
        printf("%-s\n",para);
        va_end( argp );
                                        /* ?argp??NULL */
       // return 0;
 




}
#endif

