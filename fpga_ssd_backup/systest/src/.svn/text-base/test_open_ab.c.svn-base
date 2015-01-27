/*
 *  test_open .c
 *
 *   define  for rhe function ssd_open test
 *
 *  Copyright (C) 2001 baidu.com
 *
 *  2012-11-11  create by liyang<liyang_15@baidu.com>
*/



#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "../include/api/ssd_api.h"
#include "../include/test_tools.h"
#include "../include/test_macro.h"

int main(void)
{
    	int ssd_flag =SSD_ERROR;
		enum SSD_TEST_RET_TYPE ret;
    	ret=SSD_TEST_OK;
        
	SSD_CASE_INFO("01-02","ALL","TEST open abnormal");        
        long OPEN_MAX = sysconf(_SC_OPEN_MAX);
        int fd = -1;
        long i;  

	SSD_CASE_INFO("01-02","01","OPEN THE SSD DEVICE");
        if ((ssd_flag = SSD_open(0)) != SSD_OK) {
        SSD_RESULT("excepted","actual", "else",SSD_OK,ssd_flag,0 );
        SSD_ERR("15","01","OPEN THE SSD DEVICE");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("15","01","OPEN THE SSD DEVICE");
        }
	
	    
	SSD_CASE_INFO("01-02","02","OPEN THE SSD DEVICE again");
        if ((ssd_flag = SSD_open(0)) != -SSD_ERR_NOT_CLOSED) {
        SSD_RESULT("excepted","actual", "else",SSD_ERR_NOT_CLOSED,ssd_flag,0 );
        SSD_ERR("01-02","02","OPEN THE SSD DEVICE again");
        ret = SSD_OPEN_ERROR;
        } else {
        SSD_SUCCESS("01-02","02","OPEN THE SSD DEVICE again");
        }

	SSD_CASE_INFO("01-02","03","SSD DEVICE CLOSE");
        SSD_close(0);

 	SSD_CASE_INFO("01-02","04","all fd is comsuming"); 
        /* consuming all fds */
        for (i = 0; i < OPEN_MAX - 3; i++) {
                if ( (fd = open("/dev/tty", O_RDONLY)) < 0) {
                        SSD_CASE_INFO("01-02","02","the fd is out");
                        break;
                }
        }

    	ssd_flag=SSD_open(0);
	if (ssd_flag!= -SSD_ERR_OEPN_DEV_FAILED) {
  
		SSD_RESULT("excepted","actual", "else",SSD_ERR_OEPN_DEV_FAILED,ssd_flag,0 );
        	
		SSD_ERR("01-02","04","OPEN THE SSD DEVICE ,when fd is out");
		ret = SSD_OPEN_ERROR;
	} 
    	else {
		SSD_SUCCESS("01-02","04","OPEN THE SSD DEVICE,when fd is out");
	}
 
 
	
	if (SSD_TEST_OK == ret)
		SSD_CASE_INFO("01-02","ALL","test open abnormal pass");
	else
		SSD_CASE_INFO("01-02","ALL","test open abnormal fail");
    
    return 0;
  
}

