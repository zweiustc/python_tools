/*
 *  Copyright (C) 2012,   baidu_ssd
 *  <wangyong03@baidu.com>, 2012.11
    <zhangwei29@baidu.com>, modified, 2013.8
 */
/**************************************
 * Header Include
 **************************************/
#include "ssd_api.h"
#include <bhp-lib.h>

/**************************************
 * Global variable and inline function
 **************************************/ 
int g_opened_flag = SSD_CLOSED;
int g_ssd_fd[SSD_MAX_CHANNEL];

/*for crc*/
unsigned long long table[256];
unsigned long long key;
char *crc_proc = "/var/run/ssd_crc";
FILE *crc_fp = NULL;

#define CRC_DATA 0x1LL
#define CRC_POLY 0x11edc6f41LL
#define CRC_DATA_BIT (1024 * 8)
#define CRC_POLY_BIT 33
#define SSD_ERR_CRC 40
#define CRC_BLOCK_HEADER  32

/* for cfg_flash*/
const int SPI_ADDR_CTRL       =   0x8040;
const int S6_ADDR             =   0x400000;
const int CMD_READ            =   0x01;
const int SPI_ADDR_STATUS     =   0x8048;

static inline int api_value(int value)
{
    return -abs(value);
}

/* hash id to 44 /dev/ssd files */
static inline int id2channel(struct BlockId id)
{
    return id.m_nLow % SSD_MAX_CHANNEL;
}

void u64tochar(char *pbuf, ssd_u64 num)
{
     int i;
     int hex_bit;
     for(i = 0; i < 16; i++) {
         hex_bit = num % 16;
         if(hex_bit < 10)
            *(pbuf + 15 - i) = '0' + hex_bit;
         else
            *(pbuf + 15 - i) = 'A' + hex_bit - 10;
         num = num / 16; 
     }
} 

inline void idtochar(char *pbuf, struct ssd_blockid id)
{
     u64tochar(pbuf, id.m_nHigh);
     u64tochar(pbuf + 16, id.m_nLow);
     *(pbuf + 32) = '\0';
};

/* set the struct of file stat */
inline void set_stat(struct stat *sb, struct flush_id_info *buffer)
{ 
    int block_num = (buffer->id.m_nHigh & 7) == 0 ? BT_INDEX_BLOCK_NUM : BT_DATA_BLOCK_NUM;  
    
    memset(sb, 0, sizeof(struct stat));
    sb->st_dev      = SSD_DEEPOCEAN_DEVICE_ID;//the device number of the file
    sb->st_ino      = buffer->block;//inode number, in SSD means logic position in channel
    sb->st_mode     = 0100755;      //file type & mode(permissions),default setting for SSD
    sb->st_rdev     = buffer->channel;//device number for special files, in SSD means channel
    sb->st_nlink    = 0;            //number of links, no meaning in SSD
    sb->st_uid      = 0;            //user ID of owner, no meaning in SSD
    sb->st_gid      = 0;            //group ID of owner, no meaning in SSD 
    sb->st_blksize  = SSD_BLOCK_SIZE;//best I/O block size; in SSD means flash block size
    sb->st_blocks   = block_num;    //num of 512-byte blocks; in SSD means num of flash blocks
    sb->st_size     = SSD_BLOCK_SIZE * block_num;//size in bytes for regular files
    sb->st_atime    = 0;            //time of last access, no meaning in SSD
    sb->st_mtime    = 0;            //time of last modif ication, no meaning in SSD
    sb->st_ctime    = 0;            //time of last file status change, no meaning in SSD 
}

// check status of hardware to see busy or not
static int check_status(int sleep_time)
{
    int timeout = 0;
    volatile uint64_t status1 = 0xffff; 
    volatile uint64_t status2 = 0xffff;

    while ((status1 & 0x01) || (status2 & 0x0f0000)) {
        if (sleep_time) {
            sleep(sleep_time);
        }
        timeout++;
        if (6 < timeout) {
            return -1;
        }
        status1 = ssd_reg_read(SPI_ADDR_STATUS); 
        status2 = ssd_reg_read(0x8068); 
    }

    return 0;
}

static inline int cfg_s6()
{
    int ret = check_status(1);
    if (ret != 0) {
        printf("cfg_s6 : Read status timeout!\n");
        return -1;
    }
    
    //send read command
    ssd_reg_write(SPI_ADDR_CTRL, S6_ADDR | CMD_READ);
    usleep(1000);   
    
    //read status
    sleep(1);
    ret = check_status(1);
    if (ret != 0) {
        printf("cfg_s6: Read status timeout after cfg_s6 cmd!\n");
    }

    return ret;    
}

static inline int cfg_s6_times()
{
    int i, ret;

    for (i = 0; i < 5; i++) {
        ret = cfg_s6();
        if (ret == 0)
            break;
    }

    return ret;
}

static inline void hardware_reset()
{
    ssd_reg_write(0x80f8, 0);
    ssd_reg_write(0x80f8, 1);
    ssd_reg_write(0x80e8, 1);
}
/**************************************
 * functions define for ssd operation
 **************************************/
/* open the SSD */
int SSD_open(int dev_num)
{
    int i = 0;
    char ssd_path[SSD_MAX_DEVNAME];
    int ret;

    if(g_opened_flag == SSD_OPENED) {
        ret = SSD_ERR_NOT_CLOSED;
        goto err;
    }

    /* open ssd device file from /dev/ssd0 to /dev/ssd43 */
    for(; i < SSD_MAX_CHANNEL; i++)
        g_ssd_fd[i] = -1;

    for(i = 0; i < SSD_MAX_CHANNEL; i++) {
        snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", i);
        g_ssd_fd[i] = open(ssd_path, O_RDWR);
        if(g_ssd_fd[i] < 0)
        {
            printf("SSD: /dev/ssd_Baidu%d open failed\n", i);
            break;
        }
    }

    if(i != SSD_MAX_CHANNEL) {
        ret = SSD_ERR_OEPN_DEV_FAILED;
        goto err;
    }

    /* init ssd just use fd[0] is enough */
    int open_count;
    for (open_count = 0; open_count < 5; open_count++) {
        hardware_reset();
        ret = ioctl(g_ssd_fd[0], IOCTL_OPEN, NULL);
        if (ret == SSD_OK || ret == SSD_ERR_NOT_CLOSED)
            break;
        ret = cfg_s6_times();
        if (ret != SSD_OK)
            goto err;
    }

    if (ret != SSD_OK)
        goto err;
    
    g_opened_flag = SSD_OPENED;

    /*for crc*/
    key = mod2(CRC_DATA, CRC_POLY, CRC_POLY_BIT, CRC_DATA_BIT);
    gen_table(table, key);
    
    /*create crc proc*/
    crc_fp = fopen(crc_proc, "w+");
    if(!crc_fp) {
        printf("fopen ssd_crc error\n");
    } else {
        fprintf(crc_fp, "%d\n", 0);
    }
    

    return SSD_OK;
    
err:
    while(i--) {
       close(g_ssd_fd[i]);
       g_ssd_fd[i] = -1;
    }

    return api_value(ret);
}

///read flush tables based on certain version of metadata
int SSD_read_flush(struct ssd_meta_version *meta_version)
{
    int ret;
    char ssd_path[SSD_MAX_DEVNAME];
    int local_ssd_fd = -1;
    //init buffer
    memset(meta_version->buff, 0, SSD_FLUSH_DATA_SIZE);

    //open block device 
    snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", 0);
    local_ssd_fd = open(ssd_path, O_RDWR);
    if(local_ssd_fd < 0) {
        printf("SSD: /dev/ssd_Baidu%d open failed\n", 0);
        ret = SSD_ERR_OEPN_DEV_FAILED;
        return api_value(ret);
    }
    //read device
    ret = ioctl(local_ssd_fd, IOCTL_READ_FLUSH, meta_version);
    if (SSD_OK != ret){
        printf("SSD: ioctl IOCTL_READ failed!\n");
        return api_value(ret);
    }

    close(local_ssd_fd);

    return api_value(ret);
}



/* close the SSD */
void SSD_close(int dev_num)
{
    int i;
    int ret;

    if(g_opened_flag != SSD_OPENED) {
        printf("The device has been closed!\n");
        return;
    }
    
    /* close ssd */
    g_opened_flag = 0;
    
    
    ret = ioctl(g_ssd_fd[0], IOCTL_CLOSE, NULL);
    if(ret != SSD_OK) {
        printf("close ssd error!\n");
        return;
    }

    /* close device files first, prevent failure of closing ssd success*/
    for(i = 0; i < SSD_MAX_CHANNEL; i++) {
        if(g_ssd_fd[i] >= 0) {
            close(g_ssd_fd[i]);
            g_ssd_fd[i] = -1;
        }
    }

    if(crc_fp)
        fclose(crc_fp);

    return;
}

int SSD_close_force(int dev_num)
{
    int i;
    char ssd_path[SSD_MAX_DEVNAME];
    int ret;

    /* open ssd device file from /dev/ssd0 to /dev/ssd43 */
    for(i = 0; i < SSD_MAX_CHANNEL; i++)
        g_ssd_fd[i] = -1;

    for(i = 0; i < SSD_MAX_CHANNEL; i++) {
        snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", i);
        g_ssd_fd[i] = open(ssd_path, O_RDWR);
        if(g_ssd_fd[i] < 0) {
            printf("SSD: /dev/ssd_Baidu%d open failed\n", i);
            break;
        }
    }

    if(i != SSD_MAX_CHANNEL) {
        while(i--) {
           close(g_ssd_fd[i]);
           g_ssd_fd[i] = -1;
        }
        return -SSD_ERR_NO_DEV;
    }

    /* init ssd just use fd[0] is enough */
    for(i = 0; i < 5; i++) {
        ret = ioctl(g_ssd_fd[0], IOCTL_CLOSE, NULL);
        if(ret == SSD_OK) {
            break;
        }
        printf("SSD: close failed, try_times %d\n", i);
        hardware_reset();
    }

    if(ret != SSD_OK) {
        printf("close ssd error!\n");
    } 
    else {
        printf("OK: ssd is normal closed!");
    }
    
    for(i = 0; i < SSD_MAX_CHANNEL; i++) {
        if(g_ssd_fd[i] >= 0) {
            close(g_ssd_fd[i]);
            g_ssd_fd[i] = -1;
        }
    }

    if(crc_fp)
        fclose(crc_fp);

    return ret;
}

/* flush information to flash */
int SSD_flush(int dev_num)
{
    int ret;

    if(g_opened_flag != SSD_OPENED) {
        ret = SSD_ERR_NOT_OPENED;
        goto out;
    }
    
    ret = ioctl(g_ssd_fd[0], IOCTL_FLUSH, NULL);
out:
    return api_value(ret);
}

/*** read addrtable of one channel, used for debug & test***/
int SSD_read_addrtable(int channel, char *buf)
{
    struct ssd_table_param param;
    char ssd_path[SSD_MAX_DEVNAME];
    int local_ssd_fd = -1;
    int ret;

    if(channel < 0 || channel > SSD_MAX_CHANNEL) {
        printf("error channel type!\n");
        ret = SSD_ERROR;
        goto out;
    }

    param.type = TABLE_ADDR_TYPE;
    param.channel = channel;
    param.usr_buf = buf;
    
    snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", 0);
    local_ssd_fd = open(ssd_path, O_RDWR);
    if (local_ssd_fd < 0) {
        printf("SSD: /dev/ssd_Baidu%d open failed\n", 0);
        ret = SSD_ERR_OEPN_DEV_FAILED;
        goto out;
    }
    
    ret = ioctl(local_ssd_fd, IOCTL_READ_TABLE, &param);
    
    close(local_ssd_fd);

out:
    return api_value(ret);
}

/*** read erase table of one channel, used for debug & test***/
int SSD_read_erasetable(int channel, char *buf)
{
    struct ssd_table_param param;
    char ssd_path[SSD_MAX_DEVNAME];
    int local_ssd_fd = -1;
    int ret;

    if(channel < 0 || channel > SSD_MAX_CHANNEL) {
        printf("error channel type!\n");
        ret = SSD_ERROR;
        goto out;
    }

    param.type = TABLE_ERASE_TYPE;
    param.channel = channel;
    param.usr_buf = buf;
    
    snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", 0);
    local_ssd_fd = open(ssd_path, O_RDWR);
    if(local_ssd_fd < 0) {
        printf("SSD: /dev/ssd_Baidu%d open failed\n", 0);
        ret = SSD_ERR_OEPN_DEV_FAILED;
        goto out;
    }
    
    ret = ioctl(local_ssd_fd, IOCTL_READ_TABLE, &param);

    close(local_ssd_fd);

out: 
    return api_value(ret);
}


/* utility of one channel, for test & debug */
uint64_t SSD_util_test(int dev_num, int channel)
{
    int block_used = 0;
    int i;
    ssd_u8 *buffer;
    struct erase_entry *erase;
    struct ssd_table_param param;
    int    ret;  
    uint64_t  blocks_idle = 0;
    char ssd_path[SSD_MAX_DEVNAME];
    int local_ssd_fd;
    
    local_ssd_fd = -1;
    snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", channel);
    local_ssd_fd = open(ssd_path, O_RDWR);
    if(local_ssd_fd < 0) {
        printf("SSD: /dev/ssd_Baidu%d open failed\n", channel);
        return -SSD_ERR_OEPN_DEV_FAILED;
    }

    buffer = (ssd_u8 *)malloc(SSD_TABLE_SIZE);  
    if(NULL == buffer) {
        close(local_ssd_fd);
        local_ssd_fd = -1;
        return 0;//-SSD_ERR_NULL;
    }
    
    memset(&param, 0, sizeof(struct ssd_table_param));
    param.usr_buf = buffer;
    param.type = TABLE_ERASE_TYPE;
    param.channel = channel;

    ret = ioctl(local_ssd_fd, IOCTL_READ_TABLE, &param);
        
    if(SSD_OK != ret) {
        printf("failed in read table, ret is %d!", ret);
        goto out; 
    }

    erase = (struct erase_entry *) buffer;

    erase = erase + PLANE_RESERVE_BLOCKS * 2;
    //each plane has two block reserved
    for(i = PLANE_RESERVE_BLOCKS * 2; i < SSD_MAX_PHYBLOCKS_PER_CHANNEL / 2; i++) {
        //block is used and not a bad block
        if (erase->use_flag && ~(erase->bad_flag))  
            block_used++;
        erase ++;
    }
    
    erase = erase + PLANE_RESERVE_BLOCKS * 2;
    for(i = i + PLANE_RESERVE_BLOCKS * 2; i < SSD_MAX_PHYBLOCKS_PER_CHANNEL; i++)  {
        if(erase->use_flag && ~(erase->bad_flag))
            block_used++;
        erase ++;
    }

    blocks_idle = SSD_MAX_VIRBLOCKS_PER_CHANNEL - block_used;
    
out:
    free(buffer);
    close(local_ssd_fd);
    local_ssd_fd = -1;

    //printf("channel %d free blocks are : %llu\n", channel, blocks_idle);
    //printf("channel %d free bytes are :%llu\n".channel, (blocks_idle * SSD_BLOCK_SIZE));
    return (blocks_idle * SSD_BLOCK_SIZE);
}

/* utility of the SSD */
uint64_t SSD_util(int dev_num)
{
    int channel;
    int block_used = 0;
    int i;
    ssd_u8 *buffer;
    struct erase_entry *erase;
    struct ssd_table_param param;
    int    ret;  
    uint64_t  blocks_idle = 0;
    
    if(g_opened_flag != SSD_OPENED)
        return 0;
    
    buffer = (ssd_u8 *)malloc(SSD_TABLE_SIZE);  
    if(NULL == buffer)
         return 0;
    
    memset(&param, 0, sizeof(struct ssd_table_param));
    param.usr_buf = buffer;
    param.type = TABLE_ERASE_TYPE;
    
    for(channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
        param.channel = channel;
        ret = ioctl(g_ssd_fd[channel], IOCTL_READ_TABLE, &param);
        
        if(SSD_OK != ret)
            goto out; 

        erase = (struct erase_entry *) buffer;

        erase = erase + PLANE_RESERVE_BLOCKS * 2;
        //each plane has two block reserved
        for(i = PLANE_RESERVE_BLOCKS * 2; i < SSD_MAX_PHYBLOCKS_PER_CHANNEL / 2; i++) {
            //block is used and not a bad block
            if(erase->use_flag && ~(erase->bad_flag))  
                block_used++;
            erase ++;
        }

        erase = erase + PLANE_RESERVE_BLOCKS * 2;
        for(i = i + PLANE_RESERVE_BLOCKS * 2; i < SSD_MAX_PHYBLOCKS_PER_CHANNEL; i++) {
            if(erase->use_flag && ~(erase->bad_flag))
                block_used++;
            erase ++;
        }
    }//end of for(channel = 0 to 43) 

    blocks_idle = SSD_MAX_VIRBLOCKS - block_used;
    
out:
    free(buffer);
    return (blocks_idle * SSD_BLOCK_SIZE);
}


/* delete an id */
int SSD_delete(int dev_num, struct BlockId id)
{
    int channel = id2channel(id); //just decide which /dev/ssd* to be accessed
    struct ssd_blockid ssd_id;
    int ret;
    
    if(g_opened_flag != SSD_OPENED) {
        ret = -SSD_ERR_NOT_OPENED; 
        goto out;
    }
    /* type conversion from c++ to c */
    ssd_id.m_nLow = id.m_nLow;
    ssd_id.m_nHigh = id.m_nHigh;
    ret = ioctl(g_ssd_fd[channel], IOCTL_DELETE, &ssd_id);

out:
    return api_value(ret);
}


/* write an id to ssd, the len must be 2M or 10M */
int SSD_write(int dev_num, struct BlockId id, char * buffer, size_t count, off_t offset)
{
    int channel; 
    struct ssd_write_param param;
//    struct ssd_write_retry_param param_retry;
    int ret;
    int blocks_failed_of_bad;
    ssd_u8 i = 0, error_times = 0;
    ssd_u8 fail_flag = 1;
    uint32_t actual_len = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
    /*for crc*/
    int crc, nlen;
    int crc_calc;
    int is_crc = -1;
    
    if(g_opened_flag != SSD_OPENED) {
        ret = SSD_ERR_NOT_OPENED; 
        goto out;
    }
        
    if(count != actual_len) {
        ret = SSD_ERR_LEN_PAGE_ALIGN;
        goto out;
    }
        
    if(buffer == NULL) {
        ret = SSD_ERR_NULL; 
        goto out;
    }

    /*for crc*/
    if(!crc_fp)
        is_crc = 0;
    else {
        fseek(crc_fp, 0L, SEEK_SET);
        fscanf(crc_fp, "%d\n", &is_crc);
    }

    if(1 == is_crc) {
        crc = *(int *)buffer;
        nlen = *(int *)(buffer + 4);
        if(0 != crc) {
            crc_calc = baidu_crc32_fast(buffer + CRC_BLOCK_HEADER, 0, nlen - CRC_BLOCK_HEADER, table);
            if(crc_calc != crc) {
                printf("crc error: crc_orig[%d] VS crc_calc[%d]\n", crc, crc_calc);
                ret = SSD_ERR_CRC;
                goto out;
            }
        }
    }

    channel = id2channel(id);   //just decide which /dev/ssd* to be accessed

    memset(&param, 0, sizeof(struct ssd_write_param));
    param.id.m_nLow  = id.m_nLow;
    param.id.m_nHigh = id.m_nHigh;
    param.buf = (ssd_u8 *)buffer;
    param.len = count >> SSD_BLOCK_SHIFT;

//    memset(&param_retry, 0, sizeof(struct ssd_write_retry_param));
    
    ret = ioctl(g_ssd_fd[channel], IOCTL_WRITE, &param);

    if(SSD_OK == ret)
        return SSD_OK;
/*
    blocks_failed_of_bad = ret >> 7;
    if(blocks_failed_of_bad == 0)
       goto out; 

    param_retry.id = param.id;
    param_retry.len = SSD_BLOCK_SIZE >> SSD_BLOCK_SHIFT;

    for(i = 0; i < count / SSD_BLOCK_SIZE; i++, fail_flag = fail_flag << 1) {
        if((blocks_failed_of_bad & fail_flag) == 0)
            continue;

        for(error_times = 0; error_times < MAX_RETRY_TIMES; error_times++) {
            param_retry.block_offset = i;
            param_retry.buf = (ssd_u8 *)(buffer + i * SSD_BLOCK_SIZE);
            ret = ioctl(g_ssd_fd[channel], IOCTL_WRITE_RETRY, &param_retry);
            if(SSD_OK == ret)
                break;
        }

        // achive the max write_retry times
        if(MAX_RETRY_TIMES == error_times) {
            SSD_delete(dev_num, id);
            break;
        }
    }
*/
out:
    return api_value(ret);
}

/* ssd read api, offset and len must be 8k aligned */
int SSD_read(int dev_num, struct BlockId id, char *buffer, size_t count, off_t offset)
{
    int channel = id2channel(id);   //just decide which /dev/ssd* to be accessed
    uint32_t data_size = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
    struct ssd_read_param param;
    int ret;
    int err_num;
    
    char ssd_path[SSD_MAX_DEVNAME];
    int local_ssd_fd = -1;

    if(g_opened_flag != SSD_OPENED && dev_num != MONITOR_DEV) {
        ret = SSD_ERR_NOT_OPENED; 
        goto out;
    }

    if(count % SSD_PAGE_SIZE) {
        ret = SSD_ERR_LEN_PAGE_ALIGN;
        goto out;
    }
    
    if(offset % SSD_PAGE_SIZE) {
        ret = SSD_ERR_START_PAGE_ALIGN;
        goto out;
    }

    if(offset > data_size) {
        ret = SSD_ERR_PARAM_OVERFLOW;
        goto out;
    }    
        
    if((count + offset) > data_size) {
        ret = SSD_ERR_PARAM_OVERFLOW;
        goto out;
    }

    if(buffer == NULL) {
        ret = SSD_ERR_NULL; 
        goto out;
    }

    if(count == 0)
        return SSD_OK;

    memset(&param, 0, sizeof(struct ssd_read_param));
    param.len = count >> SSD_PAGE_SHIFT;
    param.buf = (ssd_u8*)buffer;
    param.offset = offset >> SSD_PAGE_SHIFT;
    param.id.m_nLow  = id.m_nLow;
    param.id.m_nHigh = id.m_nHigh;
    
    if(dev_num == MONITOR_DEV) {
        snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", 0);
        local_ssd_fd = open(ssd_path, O_RDWR);
        if(local_ssd_fd < 0) {
            printf("SSD: /dev/ssd_Baidu%d open failed\n", 0);
            ret = SSD_ERR_OEPN_DEV_FAILED;
            goto out;
        }
    }
    

    for(err_num = 0; err_num < 3; err_num++) {
        if(dev_num == MONITOR_DEV)
            ret = ioctl(local_ssd_fd, IOCTL_READ, &param);
        else
            ret = ioctl(g_ssd_fd[channel], IOCTL_READ, &param);
     
        if (ret == SSD_OK)
            break;
    }

    if(dev_num == MONITOR_DEV)
        close(local_ssd_fd);

out:
    return api_value(ret);
}

/* trasvers all blocks in SSD */
int SSD_ftw(int dev_num, int (*fn) (const char *fpath, const struct stat *sb, int typeflag), int depth)
{
    int i;
    int ret;
    int id_num;
    struct ssd_readid_param rid_param;
    struct flush_id_info *buffer;
    struct stat file_stat;
    char pbuf[33];

    char ssd_path[SSD_MAX_DEVNAME];
    int local_ssd_fd = -1;
    
    if(g_opened_flag != SSD_OPENED && dev_num != MONITOR_DEV) {
        ret = SSD_ERR_NOT_OPENED;
        goto out;
    }
        
    buffer = (struct flush_id_info *)malloc(SSD_MAX_ID_NUM * sizeof(struct flush_id_info));
    if(buffer == NULL) {
        ret = SSD_ERR_NOMEM;
        goto out;
    }
        
    memset(buffer, 0, SSD_MAX_ID_NUM * sizeof(struct flush_id_info));
    rid_param.usr_buf = (ssd_u8 *)buffer;
    
    if(dev_num == MONITOR_DEV) {
        snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", 0);
        local_ssd_fd = open(ssd_path, O_RDWR);
        if(local_ssd_fd < 0) {
            printf("SSD: /dev/ssd_Baidu%d open failed\n", 0);
            ret = SSD_ERR_OEPN_DEV_FAILED;
            goto out;
        }
        
        id_num = ioctl(local_ssd_fd, IOCTL_READ_ID_TABLE, &rid_param);
        close(local_ssd_fd);
    } else
        id_num = ioctl(g_ssd_fd[0], IOCTL_READ_ID_TABLE, &rid_param); 
    
    //error ret: means error code
    if(id_num < 0 || id_num > SSD_MAX_ID_NUM) { 
        free(buffer);
        return id_num;
    }
    
    //normal ret: means id number in all the table
    //trasvers the hashtable of ID
    ret = SSD_OK;//if id_num = 0, set the original ret
    for(i = 0; i < id_num; i++) {
        //update stat
        set_stat(&file_stat, buffer + i);
        
        idtochar(pbuf, (buffer + i)->id);

        //FTW_F:all blocks in SSD are regular files
        ret = fn(pbuf, &file_stat, FTW_F);
        //stop trasvers
        if(ret != 0)  
            break;
    }

out:
    if(buffer)
        free(buffer);
    return api_value(ret);
}

int SSD_open_newcard(int dev_num)
{
    int i = 0;
    char ssd_path[SSD_MAX_DEVNAME];
    int ret;

    if (g_opened_flag == SSD_OPENED) {
        ret = SSD_ERR_NOT_CLOSED;
        goto err;
    }

    /* open ssd device file from /dev/ssd0 to /dev/ssd43 */
    for(; i < SSD_MAX_CHANNEL; i++)
        g_ssd_fd[i] = -1;

    for(i = 0; i < SSD_MAX_CHANNEL; i++) {
        snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", i);
        g_ssd_fd[i] = open(ssd_path, O_RDWR);
        if(g_ssd_fd[i] < 0) {
            printf("SSD: /dev/ssd_Baidu%d open failed\n", i);
            break;
        }
    }

    if(i != SSD_MAX_CHANNEL) {
        ret = SSD_ERR_OEPN_DEV_FAILED;
        goto err;
    }

    /* init ssd just use fd[0] is enough */
    ret = ioctl(g_ssd_fd[0], IOCTL_SYS_INIT, NULL);
    if(ret != SSD_OK)
        goto err;
    
    g_opened_flag = SSD_OPENED;
    return SSD_OK;
    
err:
    while(i--)
    {
       close(g_ssd_fd[i]);
       g_ssd_fd[i] = -1;
    }

    return api_value(ret);
}

uint64_t ssd_reg_read(uint64_t addr)
{
    struct ssd_regrd_param param;
    ssd_u64 read_value;
    char ssd_path[SSD_MAX_DEVNAME];
    int local_ssd_fd;
    
    /* open ssd device file /dev/ssd0*/
    local_ssd_fd = -1;
    snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", 0);
    local_ssd_fd = open(ssd_path, O_RDWR);
    if(local_ssd_fd < 0) {
        printf("SSD: /dev/ssd_Baidu%d open failed\n", 0);
        return 0;
    }
    
    param.addr = addr;
    param.ret_value = &read_value;
    (void)ioctl(local_ssd_fd, IOCTL_REG_READ, &param);
    
    close(local_ssd_fd);
    local_ssd_fd = -1;
    
    return read_value;
}

int ssd_reg_write(uint64_t addr, uint64_t value)
{
    struct ssd_regwr_param param;
    char ssd_path[SSD_MAX_DEVNAME];
    int ret;
    int local_ssd_fd;
    
    /* open ssd device file /dev/ssd0*/
    local_ssd_fd = -1;
    snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", 0);
    local_ssd_fd = open(ssd_path, O_RDWR);
    if(local_ssd_fd < 0) {
        printf("SSD: /dev/ssd_Baidu%d open failed\n", 0);
        ret = SSD_ERR_OEPN_DEV_FAILED;
        goto out;
    }
    

    param.addr  = addr;
    param.value = value;
    ret = ioctl(local_ssd_fd, IOCTL_REG_WRITE, &param);
    
    close(local_ssd_fd);
    local_ssd_fd = -1;

out:    
    return api_value(ret);
}

int SSD_mark_bad_block(int dev_num, int type, struct BlockId id, ssd_u16 block, ssd_u16 channel)
{
    struct ssd_mark_bad_param mbparam;
    int id_channel, ret, err_num;

    char ssd_path[SSD_MAX_DEVNAME];
    int local_ssd_fd = -1;

    if(g_opened_flag != SSD_OPENED && dev_num != MONITOR_DEV) {
        ret = SSD_ERR_NOT_OPENED; 
        goto out;
    }

    memset(&mbparam, 0, sizeof(struct ssd_mark_bad_param));

    if(PARAM_ID == type) {
        mbparam.type = PARAM_ID;
        mbparam.u.id.m_nHigh = id.m_nHigh;
        mbparam.u.id.m_nLow = id.m_nLow;
        id_channel = id2channel(id);
    }

    if(PARAM_BLOCK == type) {
        if(channel >= SSD_MAX_CHANNEL || block >= SSD_MAX_VIRBLOCKS_PER_CHANNEL) {
            printf("error channel type!\n");
            ret = SSD_ERROR;
            goto out;
        }

        mbparam.type = PARAM_BLOCK;
        mbparam.u.param_block.block = block;
        mbparam.u.param_block.channel = channel;
        id_channel = channel;
    }

    if(dev_num == MONITOR_DEV) {
        snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", 0);
        local_ssd_fd = open(ssd_path, O_RDWR);
        if (local_ssd_fd < 0) {
            printf("SSD: /dev/ssd_Baidu%d open failed\n", 0);
            ret = SSD_ERR_OEPN_DEV_FAILED;
            goto out;
        }
    }

    for(err_num = 0; err_num < 3; err_num++) {
        if (dev_num == MONITOR_DEV)
            ret = ioctl(local_ssd_fd, IOCTL_MARK_BAD_BLOCK, &mbparam);
        else
            ret = ioctl(g_ssd_fd[id_channel], IOCTL_MARK_BAD_BLOCK, &mbparam);
     
        if (ret == SSD_OK)
            break;
    }

    if (dev_num == MONITOR_DEV)
        close(local_ssd_fd);
out:
    return api_value(ret);
}

int SSD_mark_erase_count(int dev_num, struct erase_entry_map *buffer, ssd_u16 count, ssd_u8 channel)
{
    int ret = 0, err_num;
    struct ssd_mark_erase_count_param mecparam;

    if(g_opened_flag != SSD_OPENED) {
       	ret = SSD_ERR_NOT_OPENED;
        goto out;
    }

	memset(&mecparam, 0, sizeof(struct ssd_mark_erase_count_param));

    mecparam.channel = channel;
    mecparam.count = count;
    mecparam.erase_buffer = (ssd_u8 *)buffer;

    for(err_num = 0; err_num < 3; err_num++) {
        ret = ioctl(g_ssd_fd[channel], IOCTL_MARK_ERASE_COUNT, &mecparam);
        if(ret == SSD_OK)
            break;
    }

out:
    return api_value(ret);
}

int SSD_recover_open(int dev_num)
{
    int i = 0;
    char ssd_path[SSD_MAX_DEVNAME];
    int ret;

    /*if(g_opened_flag == SSD_OPENED) {
        ret = SSD_ERR_NOT_CLOSED;
        goto err;
    }*/

    /* open ssd device file from /dev/ssd0 to /dev/ssd43 */
    for(; i < SSD_MAX_CHANNEL; i++)
        g_ssd_fd[i] = -1;

    for(i = 0; i < SSD_MAX_CHANNEL; i++) {
        snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", i);
        g_ssd_fd[i] = open(ssd_path, O_RDWR);
        if(g_ssd_fd[i] < 0)
        {
            printf("SSD_recover: /dev/ssd_Baidu%d open failed\n", i);
            break;
        }
    }

    if(i != SSD_MAX_CHANNEL) {
        ret = SSD_ERR_OEPN_DEV_FAILED;
        goto err;
    }

    /* init ssd just use fd[0] is enough */
    int open_count;
    for (open_count = 0; open_count < 5; open_count++) {
        hardware_reset();
        ret = ioctl(g_ssd_fd[0], IOCTL_RECOVER_OPEN, NULL);
        if (ret == SSD_OK || ret == SSD_ERR_NOT_CLOSED)
            break;
        ret = cfg_s6_times();
        if (ret != SSD_OK)
            goto err;
    }

    printf("ssd_recover: ret %d, count %d\n", ret, open_count);

    if (ret != SSD_OK)
        goto err;
    
    g_opened_flag = SSD_OPENED;

    /*for crc*/
    key = mod2(CRC_DATA, CRC_POLY, CRC_POLY_BIT, CRC_DATA_BIT);
    gen_table(table, key);
    
    /*create crc proc*/
    crc_fp = fopen(crc_proc, "w+");
    if(!crc_fp) {
        printf("fopen ssd_crc error\n");
    } else {
        fprintf(crc_fp, "%d\n", 0);
    }
    

    return SSD_OK;
    
err:
    while(i--) {
       close(g_ssd_fd[i]);
       g_ssd_fd[i] = -1;
    }

    return api_value(ret);
}

/*----------------------------------------------------------------------------*
REV #  DATE       BY    REVISION DESCRIPTION
-----  --------  -----  ------------------------------------------------------
0001   11/10/24   wangyong   Initial version.
0002   13/07/09   suijulei   Version 1-0-0-5
*----------------------------------------------------------------------------*/

