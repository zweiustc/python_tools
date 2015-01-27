#ifndef __SSD_TEST_MACRO_H__
#define __SSD_TEST_MACRO_H__

//#include "../include/api/ssd_api.h"
//#include "../include/api/ssd_api.c"

/*********  test return status ************/
enum SSD_TEST_RET_TYPE{
	SSD_TEST_OK = 0,
	SSD_OPEN_ERROR,
	SSD_CLOSE_ERROR,
	SSD_FLUSH_ERROR,
	SSD_UTIL_ERROR,
    SSD_FTW_ERROR,
	SSD_READ_ERROR,
	SSD_WRITE_ERROR,
	SSD_DELETE_ERROR,
	SSD_BACKUP_ERROR,
	SSD_MAJOR_ERROR,
	SSD_CHANNEL_ERROR,
	SSD_INFO_ERROR,
	SSD_INIT_ERROR,
	SSD_TEST_ERROR = 100,
};

 
#endif

