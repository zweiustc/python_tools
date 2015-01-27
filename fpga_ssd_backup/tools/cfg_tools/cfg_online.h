#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include "ssd_api.h"

//parameter of configure online 
#define SPI_ADDR_CTRL           0x8040
#define SPI_ADDR_STATUS         0x8048
#define SPI_RAM_START           0x8100

#define S6_START_ADDR           0x530000

//command of configure online
#define CMD_READ                0x01
#define CMD_WRITE               0x02
#define CMD_ERASE               0x04

//write 256byte to ram each time
#define MAXLEN                  256

//time out setting
#define SPI_TIME_READ_STATUS    5
#define SPI_TIME_ERASE          300
//#define SPI_TIME_CFG_S6         

//file format setting
#define BUF_SIZE                50
#define EMPTY_BYTE              0xFF
#define MAX_FLASH_ADDR          0x1000000

//three versions of S6 bit address
#define S6_ADDR1                0x180000
#define S6_ADDR2                0x600000
#define S6_ADDR3                0xA80000

 
enum CFG_ERROR_TYPE{
    CFG_OK                  = 0,
    CFG_READ_STATUS_TIMEOUT = 1,
    CFG_ERASE_TIMEOUT       = 2,
    CFG_WRITE_TIMEOUT       = 3,
    CFG_CFG_S6_TIMEOUT      = 4,
    CFG_NO_FILE             = 5,
    CFG_PARAM_ERROR         = 6,
};


int file_change(const char* mcs_pathname, const char* bin_pathname);
int cfg_flash(const char* bin_pathname);
int cfg_s6(uint64_t addr);





