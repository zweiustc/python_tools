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

struct BlockId store_write[8000];
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
};


void gen_order_id(const int num,const uint64_t step)
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
		
        len=rand()%2;      
        gen_next_id(id,&next_id,len,step);
        id=next_id;
    }
    
}



void gen_randm_id(const int num)
{
  int i,j;  
  for(i=0;i<num;i++){
      store_write_temp[i].m_nLow=0;
  }
 
  for(i=0;i<num;i++){
      j=rand()%num;
      while(store_write_temp[j].m_nLow != 0){
          j=rand()%num;
      }
      store_write_temp[j]=store_write[i];
	  store_read_len_temp[j]=store_read_len[i];
	  store_offset_temp[j]=store_offset[i];
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


int read_process_com(const struct BlockId id,  char * const buffer,\
                 const uint32_t offset,const uint32_t len,const int status)
{


        int flag;
        if ((flag = SSD_read(0,id, buffer, offset,len)) != status) {
        SSD_RESULT("excepted","actual", "else",status,flag,0 );
        SSD_RESULT("id.m_nLow","len", "offset",id.m_nLow,len,offset);
        SSD_ERR("r","0","SSD_read");
        } else {
        SSD_RESULT("id.m_nLow","len", "offset",id.m_nLow,len,offset);
        SSD_SUCCESS("r","0","SSD_read");
        }
	return flag;
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
    printf(" thread ssd_flush()\n");
    flag= SSD_flush(0);
    printf(" SSD_flush successful");
    return (void *)0;
}


//===================write-read-delete--

 void *
thr_fn_write_read_delete(void* arg)
{


        int i,j,m;
        struct thread_coeff * para ;
        para=( struct thread_coeff * )arg;
        BlockId id;

        uint32_t len;
        uint32_t offset;

        const int BUFFSIZE=BT_DATA_SIZE; //100M
        char * buff;
        buff = (char *)malloc(BUFFSIZE*sizeof(char));//
        if (NULL == buff) {
                printf("can not malloc buff in read_ok\n");
        exit(-1);
        }



        j=para->start_num;
        m=para->step;
        for (i=0;i<para->id_num;i++){
                id=store_write_temp[j+i*m];
                len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
                write_process(id,buff,len,SSD_OK);

                offset=store_offset_temp[j+i*m];//get_randm_offset(id);
                len=store_read_len_temp[j+i*m];//get_randm_read_len(id,offset);
                read_process(id,buff,offset,len,SSD_OK);
				if(id.m_nLow%10==1){
                delete_process(id,SSD_OK);
				}
        }



        free(buff);

        return (void *)0;
}


 void *
thr_fn_compare(void* arg)
{


        int i,j,m;
        struct thread_coeff * para ;
        para=( struct thread_coeff * )arg;
        BlockId id,next_id;

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
		
	for(i = 0; i < 2*BT_DATA_SIZE; i++)
		*(buff_w+i) = rand();

		
	memset(buff_r, 0, BUFFSIZE);

       // j=para->start_num;
        m=para->step;
		
	id=para->start_id;
        for (i=0;i<para->id_num;i++){
                
				
		len=rand()%2;
		gen_next_id(id,&next_id,len,m);
		id=next_id;
		len=(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
		write_process(id,buff_w,len,SSD_OK);		
		
		memset(buff_r, 0, BUFFSIZE);
                offset=get_randm_offset(id);
                len=get_randm_read_len(id,offset);
                read_process(id,buff_r,offset,len,SSD_OK);
				
				 
		if (compare_m(buff_w+offset, buff_r, len) != 0 ) {
						 
			SSD_RESULT("id.m_nLow","read_len", "offset",id.m_nLow,len,offset);
						 
			SSD_ERR("c","0","SSD_data_compare");
										      
			}
		else
			{
			SSD_SUCCESS("w","0","SSD_data_compare");
			}
				
		if(id.m_nLow%10==1){
                	delete_process(id,SSD_OK);
				
			}
        }



        free(buff_w);
		
	free(buff_r);

        return (void *)0;
}

 void *
thr_fn_compare_divide(void* arg)
{


        int i,j,m,test_time;
        struct thread_coeff * para ;
        para=( struct thread_coeff * )arg;
        BlockId id,next_id,first_id;

        uint32_t len;
	uint32_t len_tmp[para->id_num];
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

       // for(i = 0; i < 2*BT_DATA_SIZE; i++)
       //         *(buff_w+i) = rand();


        memset(buff_r, 0, BUFFSIZE);

       // j=para->start_num;
        m=para->step;

        id=para->start_id;
	first_id =id;


	test_time = para->id_num;

        for (i=0;i<test_time;i++){


                //len=rand()%2;
		memset(buff_w, i, BUFFSIZE);
		len_tmp[i]=len;
                gen_next_id(id,&next_id,1,m);
                id=next_id;
                len= BT_DATA_SIZE;
                write_process(id,buff_w,len,SSD_OK);

	}

	id=first_id;
       for (i=0;i<test_time;i++){


                gen_next_id(id,&next_id,1,m);
                id=next_id;
                memset(buff_r, 0, BUFFSIZE);
		memset(buff_w, i, BUFFSIZE);
                offset=get_randm_offset(id);
                len=get_randm_read_len(id,offset);
                read_process(id,buff_r,offset,len,SSD_OK);


                if (compare_m(buff_w+offset, buff_r, len) != 0 ) {

                        SSD_RESULT("id.m_nLow","read_len", "offset",id.m_nLow,len,offset);

                        SSD_ERR("c","0","SSD_data_compare");

                        }
                else
                        {
                        SSD_SUCCESS("w","0","SSD_data_compare");
                        }

/*                if(id.m_nLow%10==1){
                        delete_process(id,SSD_OK);

                        }*/
        }	



        free(buff_w);

        free(buff_r);

        return (void *)0;
}

 void *
thr_fn_all_operation(void* arg)
{


        int i,j,m;
        struct thread_coeff * para ;
        para=( struct thread_coeff * )arg;
        BlockId id,next_id;

        uint32_t len;
        uint32_t offset;
	uint32_t ssd_flag;
	uint64_t util_flag;

        const int BUFFSIZE=BT_DATA_SIZE; //100M
        char * buff_w;
                
	char * buff_r;
        buff_r = (char *)malloc(BUFFSIZE*sizeof(char));//
	buff_w = (char *)malloc(BUFFSIZE*sizeof(char));//
        if ((NULL == buff_r)||(NULL ==buff_w)) {
                printf("can not malloc buff in read_ok\n");
                        exit(-1);
        }

//        for(i = 0; i < BT_DATA_SIZE; i++)
//                *(buff_w+i) = rand();

       memset(buff_r, 0, BUFFSIZE);
	printf("it's OK");
       // j=para->start_num;
        m=para->step;

        id=para->start_id;

        for (i=0;i<para->id_num;i++){


                len=rand()%2;
                gen_next_id(id,&next_id,len,m);
                id=next_id;
		len=(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
		write_process(id,buff_w,len,SSD_OK);

                memset(buff_r, 0, BUFFSIZE);
                offset=0;//get_randm_offset(id);
                len=(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;//get_randm_read_len(id,offset);
                read_process(id,buff_r,offset,len,SSD_OK);

                if (compare_m(buff_w, buff_r, len) != 0 ) {

                        SSD_RESULT("id.m_nLow","read_len", "offset",id.m_nLow,len,offset);

                        SSD_ERR("c","0","SSD_data_compare");

                        }
		else{
		SSD_SUCCESS("w","0","SSD_data_compare");	
		}

                if(id.m_nLow%10==1){
                        delete_process(id,SSD_OK);

                        }

		if(id.m_nLow%20==1){
			if ((ssd_flag = SSD_flush(0)) != SSD_OK) {
                		printf("\tERROR!\n");
                		printf("\texcepted: %d, actual: %d\n", SSD_OK, ssd_flag);
                		exit(-1);
        		} else {
                		printf(" SSD_flash returns successfully\n");
        		}
		}
		
 		if(id.m_nLow%40==1){
			if ((util_flag = SSD_util(0)) != 0) {
                		SSD_RESULT("excepted","actual", "else",0,ssd_flag,0 );
        		} else {
                		SSD_SUCCESS("","","SSD_util");
        		}
		}

        }



        free(buff_w);

        free(buff_r);

        return (void *)0;
}


 void *
thr_fn_all_operation_random(void* arg)
{


        int i,j,m;
        struct thread_coeff * para ;
        para=( struct thread_coeff * )arg;
        BlockId write_id,read_id,next_id,first_id;
	uint32_t write_time,read_time;
        uint32_t len;
	uint32_t len_tmp[20];
        uint32_t offset;
        uint32_t ssd_flag;
        uint64_t util_flag;

        const int BUFFSIZE=BT_DATA_SIZE; //10M
        char * buff_w;

        char * buff_r;
        buff_r = (char *)malloc(BUFFSIZE*sizeof(char));//
	buff_w = (char *)malloc(BUFFSIZE*sizeof(char));//
        
	if ((NULL == buff_r)||(NULL ==buff_w)) {
                printf("can not malloc buff \n");
                        exit(-1);
        }

        //for(i = 0; i < 2*BT_DATA_SIZE; i++)
        //        *(buff_w+i) = rand();

	//memset(buff_w,0xff,sizeof(buff_w));
////        memset(buff_r, 0, BUFFSIZE);

       // j=para->start_num;
        
	m=para->step;
        write_id=para->start_id;
        //in the func,id_num is the team of the id 
	for (i=0;i<para->id_num/10;i++){

		write_time=rand()%10+1;
		for(j=0;j<write_time;j++){
		
			if(j==0)
			first_id =write_id;

			len=(write_id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
			write_process(write_id,buff_w,len,SSD_OK);
			len_tmp[j]=rand()%2;
                        gen_next_id(write_id,&next_id,len_tmp[j],m);
                        write_id=next_id;
			
		}
		
		read_time=rand()%write_time+1;
		read_id=first_id;
		for(j=0;j<write_time;j++){
                	memset(buff_r, 0, BUFFSIZE);
                	offset=0;//get_randm_offset(id);
                	len=(read_id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;//get_randm_read_len(id,offset);
			
                		read_process(read_id,buff_r,offset,len,SSD_OK);
			
                		if (compare_m(buff_w+offset, buff_r, len) != 0 ) {
                        		SSD_RESULT("id.m_nLow","read_len", "offset",read_id.m_nLow,len,offset);
                        		SSD_ERR("c","0","SSD_data_compare");
                        	}
				else
				{
				SSD_SUCCESS("w","0","SSD_data_compare");
				}

                	if(read_id.m_nLow%5==1){
                        	delete_process(read_id,SSD_OK);
                        }

                	if(read_id.m_nLow%20==1){
                        	if ((ssd_flag = SSD_flush(0)) != SSD_OK) {
                                	printf("\tERROR!\n");
                                	printf("\texcepted: %d, actual: %d\n", SSD_OK, ssd_flag);
                                	exit(-1);
                        	} else {
                                	printf(" SSD_flash returns successfully\n");
                        		}
                	}

			gen_next_id(read_id,&next_id,len_tmp[j],m);
                        read_id=next_id;

                	if(read_id.m_nLow%10==1){
                        	if ((util_flag = SSD_util(0)) != 0) {
                                	SSD_RESULT("excepted","actual", "else",0,ssd_flag,0 );
                        	} else {
                                	SSD_SUCCESS("","","SSD_util");
                        		}
                		}
		
		}

        }



        free(buff_w);

        free(buff_r);

        return (void *)0;
}




//==========================================read===================

 void *
thr_fn_read(void* arg)
{
	 

	int i,j,m;
    	struct thread_coeff * para ;
    	para=( struct thread_coeff * )arg;   
    	BlockId id;
  
    	uint32_t len;
		uint32_t offset;

    	const int BUFFSIZE=BT_DATA_SIZE; //100M
		char * buff;
		buff = (char *)malloc(BUFFSIZE*sizeof(char));//
		if (NULL == buff) {
			printf("can not malloc buff in read_ok\n");
        	exit(-1);
		}


    
     	j=para->start_num;
     	m=para->step;	
     	for (i=0;i<para->id_num;i++){
        	id=store_write_temp[j+i*m];
    		offset=store_offset_temp[j+i*m];//get_randm_offset(id);
    		len=store_read_len_temp[j+i*m];//get_randm_read_len(id,offset);
    		read_process(id,buff,offset,len,SSD_OK);
      	}
    	
    	
 
    	free(buff);

	return (void *)0;
}

void *
thr_fn_read_ab(void *arg)
{

    
	struct BlockId * id ;
    	
	id=( struct BlockId * )arg;
	BlockId id_cur = *id;
	BlockId next_id;
    	
	uint32_t len = 0;
	uint32_t offset;

    
    	
	const int BUFFSIZE = 100 * 1024 * 1024; //100M
	char *buff;
    	
	char *err_buff;
	buff = (char *)malloc(BUFFSIZE*sizeof(char));//
	if (NULL == buff) {
		printf("can not malloc buff in read_ok\n");
        exit(-1);
	}
	
   
    	    	
    /* read id is not exist */
	printf("case 10-04: read,but id is not exist \n");
	gen_next_id(id_cur,&next_id,2,1);
	id_cur=next_id;
	offset=get_randm_offset(id_cur);
    	
	len=get_randm_read_len(id_cur,offset);
    	
	read_process( id_cur,  buff, offset,len,SSD_OK);
    
   /* read len error */
	printf("case 10-05: read,but len is error \n");
	id_cur=*id;
    	
	len=0xffffffff;
	offset=get_randm_offset(id_cur);
    	read_process( id_cur,  buff, offset,len,SSD_ERR_LEN_PAGE_ALIGN);
  
  
  /* read offset error */
	printf("case 10-06: read,but len is error \n");
	id_cur=*id;
    	
	len=8*1024;
	offset=0xffffffff;
    	
	read_process( id_cur,  buff, offset,len,SSD_ERR_START_PAGE_ALIGN);


		/* buff < len  */
	printf("case 10-07: write,but buff   is shorter \n");
		err_buff = (char *)realloc(buff, 2*1024*1024);//2MB
		if(NULL == err_buff){
			printf("can not realloc, buff remains 10MB\n");
			//ret = SSD_TEST_ERROR;
		}else{
			buff = err_buff;
			err_buff = NULL;
		}	
	id_cur=*id;
	offset=get_randm_offset(id_cur);
    	
	len=get_randm_read_len(id_cur,offset);
    	
	read_process( id_cur,  buff, offset,len,SSD_OK);///to be changed SSD_OK
    	
 
	free(buff);
	free(err_buff);

	return (void *)0;
}
 
 //========================================write====================


 void *
thr_fn_write(void* arg)
{
	 

	int i,j,m;
	struct thread_coeff * para ;
	para=( struct thread_coeff * )arg;   
	BlockId id;
	uint32_t len=0;
	const int BUFFSIZE=BT_DATA_SIZE; //100M
	char * buff;
	buff = (char *)malloc(BUFFSIZE*sizeof(char));//
	if (NULL == buff) {
		printf("can not malloc buff in read_ok\n");
        exit(-1);
	}


    
     	j=para->start_num;
     	m=para->step;
     	for (i=0;i<para->id_num;i++){
        	id=store_write_temp[j+i*m];
                
			len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
    		write_process(id,buff,len,SSD_OK);
      	}
    	
    	
 
    	free(buff);

	return (void *)0;
}

 void *
thr_fn_write_full(void* arg)
{
	 

	int i,m;
    	
	struct thread_coeff * para ;
    	
	para=( struct thread_coeff * )arg;   
    	
	BlockId id;
  
    	
	uint32_t len=0;

    	
	const int BUFFSIZE=BT_DATA_SIZE; //100M
	char * buff;
	buff = (char *)malloc(BUFFSIZE*sizeof(char));//
	if (NULL == buff) {
		printf("can not malloc buff in read_ok\n");
        exit(-1);
	}


    
     	id = para->start_id;
     	m=para->step;
     	for (i=0;i<para->id_num;i++){
		len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
    		write_process(id,buff,len,SSD_OK);
		id.m_nLow = id.m_nLow + m ;
      	}
    	
    	
 
    	free(buff);

	return (void *)0;
}


    
void *
thr_fn_write_ab(void * arg)
{
    
    	
	int i;
	uint32_t len = 0;

    	struct BlockId * id ;
    	id=( struct BlockId * )arg;
    
	BlockId id_cur =*id;
    	
	BlockId next_id; 
	char *buff;
    	char *err_buff,*null_buff;
    	const int BUFFSIZE = BT_DATA_SIZE; //10m

	buff = (char *)malloc(BUFFSIZE*sizeof(char));
	if (NULL == buff) {
		printf("\tERROR! can not malloc buff in write_ok\n");
        exit(-1);
	}

	for (i = 0; i < BUFFSIZE; i++) {
		*(buff+i) = rand();
	}
	  
	  /* test the write        */
  

  
  /* write same id */
	printf("case : write same id \n");
	id_cur=*id;
	len = (id_cur.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
    	write_process( id_cur,  buff, len,SSD_OK);
    	write_process( id_cur,  buff, len,SSD_ID_ALREADY_EXIST); //to be changed
  
         /* write len is not 2m or 10m */
        printf("case : write same id \n");
        id_cur=*id;
        len = (id_cur.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
        len = len+1;
	
		write_process( id_cur,  buff, len,SSD_ERR_START_BLOCK_ALIGN); //to be changed
        
         /* write len and id not the same */
        printf("case : write same id \n");
        
        id_cur=*id;
        gen_next_id(id_cur,&next_id,0,1);
        id_cur=next_id;
        len = BT_DATA_SIZE;
        write_process( id_cur,  buff, len,SSD_OK); //to be changed 
        gen_next_id(id_cur,&next_id,1,1);
        id_cur=next_id;
        len = BT_INDEX_SIZE;
        write_process( id_cur,  buff, len,SSD_OK); //to be changed 



        printf("case : buff is null \n");
        null_buff=NULL;
        gen_next_id(id_cur,&next_id,1,1);
        id_cur=next_id;
        len=(id_cur.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
        write_process( id_cur,  null_buff, len, SSD_OK);
        
		/* buff < len  */
	
		printf("case : write,but buff   is shorter \n");
	
		err_buff = (char *)realloc(buff, 2*1024*1024);//2MB
	
		if(NULL == err_buff){
		
				printf("can not realloc, buff remains 10MB\n");
			//ret = SSD_TEST_ERROR;
	
		}else{
		
				buff = err_buff;
		
				err_buff = NULL;
	
		}	
	
		gen_next_id(id_cur,&next_id,1,1);
	
		id_cur=next_id;
        	len = (id_cur.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
    		write_process( id_cur,  buff, len,SSD_OK);        //to be changed
  
		
  /* buff > len  */
	
		printf("case : write,but buff   is shorter \n");
	
		err_buff = (char *)realloc(buff, 10*1024*1024);//2MB
	
		if(NULL == err_buff){
		printf("can not realloc, buff remains 10MB\n");
	//	ret = SSD_TEST_ERROR;
	
		}else{
		buff = err_buff;
		err_buff = NULL;
	
		}	
	
		gen_next_id(id_cur,&next_id,0,1);
	
		id_cur=next_id;
	
		len = (id_cur.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
     		write_process( id_cur,  buff, len,SSD_OK);        //to be changed

	
		free(buff);
     		free(err_buff);
	
		return (void *)0;
}

 
//======================================delete====================== 




 void *
thr_fn_delete(void* arg)
{
	 

	
		int i,j,m;
    		struct thread_coeff * para ;
    		para=( struct thread_coeff * )arg;   
    		BlockId id;

    		const int BUFFSIZE=BT_DATA_SIZE; //100M
	
		char * buff;
	
		buff = (char *)malloc(BUFFSIZE*sizeof(char));//
	
		if (NULL == buff) {
		printf("can not malloc buff in read_ok\n");
        	exit(-1);
	
		}


    
     		j=para->start_num;
     		m=para->step;
     		for (i=0;i<para->id_num;i++){
        	id=store_write_temp[j+i*m];
    		delete_process(id,SSD_OK);
      		}
    	
    	
 
    		free(buff);

		return (void *)0;
}





void *
thr_fn_delete_ab(void * arg)
{
	
     	struct BlockId * id ;
     	id=( struct BlockId * )arg;
     
	BlockId id_cur=*id;
	BlockId next_id;

      
	/* delete id is not exist */
	printf(" delete,but id is not exist \n");
    	
	int len=rand()%2;
	gen_next_id(id_cur,&next_id,len,1);
	
    	
	delete_process( id_cur,SSD_OK);
  
   

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
    	write_len=rand()%2;
    
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
//	srand((int)time(0));
    	
	write_len=(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
	offset=random_m(write_len)/(8*1024)*(8*1024);
	return offset;
	
	
	
}
	
		
 uint32_t get_randm_read_len(const struct BlockId id,uint32_t offset) //1024*102
{
	
		uint32_t i,read_len,write_len;
    	int temp;
    	write_len=(id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
	
//		srand((int)time(0));
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

int delete_all_add_id( const char *file ,const  struct stat *sb , int flag)

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
	
	if(id.m_nLow >=1500*44){
        delete_process(id,SSD_OK);
	}
        }

    return 0;

}


int delete_some_id( const char *file ,const  struct stat *sb , int flag)

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
	
	if(id.m_nLow%31==0){
        delete_process(id,SSD_OK);
	}
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

int print_all_add_id(const char *file ,const struct stat *sb ,int flag)

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

        strncpy(low,name+16,8);
        low_hex=strtol(low,NULL,16);
        strncpy(low,name+24,8);
        low_hex=(low_hex<<32)+ strtol(low,NULL,16);
        id.m_nLow=low_hex;
	if(id.m_nLow >=1500*44){
	
	printf(" HIGE 64bits of the id:%lx",id.m_nHigh);
        printf(" Low 64bits of the id:%lx\n",id.m_nLow);
	}

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

