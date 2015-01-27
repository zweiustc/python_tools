/*
 *  h/ssd_api.h
 *
 *   define user api for ssd operation
 *
 *  Copyright (C) 2001 baidu.com
 *
 *  2012-10-31  create by wangyong<wangyong03@baidu.com>
*/

#ifndef SSD_API_INCLUDED
#define SSD_API_INCLUDED

/**************************************
 * Header Include
 **************************************/
#include <stdint.h>
#include <ftw.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>

/**************************************
 *      Constants 
 **************************************/
#define SSD_MAX_CHANNEL                 44
#define SSD_MAX_DEVNAME                 16
#define SSD_CLOSED                      0
#define SSD_OPENED                      1
#define PLANE_RESERVE_BLOCKS            0x5
#define SSD_TABLE_SIZE                  (16 * 1024)
#define SSD_MAX_VIRBLOCKS_PER_CHANNEL   8000
#define SSD_MAX_PHYBLOCKS_PER_CHANNEL   8192
#define SSD_MAX_VIRBLOCKS               (SSD_MAX_CHANNEL * SSD_MAX_VIRBLOCKS_PER_CHANNEL)
#define SSD_PAGE_SIZE                   8192
#define SSD_PAGES_PER_BLOCK             256
#define SSD_BLOCK_SIZE                  (SSD_PAGE_SIZE * SSD_PAGES_PER_BLOCK)
#define SSD_BLOCKS_PER_FLUSH_DATA       5
#define SSD_FLUSH_DATA_SIZE             (SSD_BLOCK_SIZE * SSD_BLOCKS_PER_FLUSH_DATA)
#define MAX_RETRY_TIMES                 5
#define RESERVE_BLOCKS_PER_CHANNEL      20
#define SSD_PAGE_SHIFT                  13 
#define SSD_BLOCK_SHIFT                 21
/* the pci device id of baidu SSD device */
#define SSD_DEEPOCEAN_DEVICE_ID         0x0007
#define BT_INDEX_BLOCK_NUM              1
#define BT_DATA_BLOCK_NUM               5
#define BT_INDEX_SIZE                   (BT_INDEX_BLOCK_NUM * SSD_BLOCK_SIZE)
#define BT_DATA_SIZE                    (BT_DATA_BLOCK_NUM * SSD_BLOCK_SIZE)
#define SSD_MAX_ID_NUM                  SSD_MAX_VIRBLOCKS

#define MONITOR_DEV                     0x010000

/**************************************
 *      Data types
 **************************************/ 
typedef unsigned char       ssd_u8;
typedef unsigned short      ssd_u16;
typedef unsigned int        ssd_u32;
typedef unsigned long long  ssd_u64;
typedef signed char         ssd_s8;
typedef short               ssd_s16;
typedef int                 ssd_s32;
typedef long long           ssd_s64;

/* id struct */
struct ssd_blockid
{
    ssd_u64 m_nLow;  
    ssd_u64 m_nHigh;
};

/* param struct of ssd write */
struct ssd_write_param
{
    struct ssd_blockid id;
    ssd_u8*  buf;         
    ssd_u32  len; 
};

/* param struct of ssd read */
struct ssd_read_param
{
    struct ssd_blockid id;
    ssd_u8*  buf;         
    ssd_u32  len; 
    ssd_u32  offset;
};

/* param struct of ssd write retry */
/*
struct ssd_write_retry_param
{
    struct ssd_blockid id;
    ssd_u8*  buf;         
    ssd_u32  len; 
    ssd_u8  block_offset;   // 0 to 4 
};
*/
struct ssd_readid_param
{
    ssd_u8* usr_buf;
};

enum TABLE_TYPE {
    TABLE_ERASE_TYPE = 0,
    TABLE_ADDR_TYPE  = 1,
    TABLE_TYPE_COUNT = 2,
};


/* param struct of table read */
struct ssd_table_param
{
    ssd_u8* usr_buf;    
    ssd_u8 channel;
    ssd_u8 type;  /* table type */
};

/* param struct of addr table */
struct addr_entry
{
    ssd_u16  pb_addr  :13;
    ssd_u16  reserved : 3;
};

/* param struct of erase table */
struct erase_entry
{
    ssd_u16  erase_times :13;
    ssd_u16  reserved    : 1;
    ssd_u16  use_flag    : 1;
    ssd_u16  bad_flag    : 1;
};

struct id_entry
{
    struct ssd_blockid id;
    ssd_u32 addr;      //有效位1位, 冲突链的下一个位置19位
    ssd_u8 channel;    //通道号:6位, 有效位1位
};

struct ssd_regwr_param
{
    ssd_u64 addr;
    ssd_u64 value;
};

struct ssd_regrd_param
{
    ssd_u64 addr;
    ssd_u64 *ret_value;
};

// meta version struct
struct ssd_meta_version
{
    ssd_u16 block;
    ssd_u8 page;
    ssd_u64 version;
    ssd_u8 *buff;
    ssd_u8 version_flag;
};

struct ssd_block_param
{
    ssd_u8 channel;
    ssd_u16 phy_block;
};

struct flush_id_info
{
    struct ssd_blockid id;
    ssd_u32 channel  : 6;   /* channel = 0...43 */
    ssd_u32 block    :13;   /* block = 0...19   */
    ssd_u32 len      : 1;   /* 0=2M, 1=10M      */
    ssd_u32 crc32;
};

#define PARAM_ID 0
#define PARAM_BLOCK 1

struct ssd_mark_bad_param
{
    ssd_u8 type; //0 param_id, 1 param_block
    union {
        struct ssd_blockid id;       
        struct {
            ssd_u16 block;
            ssd_u16 channel;
            ssd_u32 pad[3];
        } param_block;
    } u;
};

/*just for set erase count*/
struct erase_entry_map
{
    ssd_u16 block;
    struct erase_entry ee;
};

/*erase_buffer(struct erase_entry_map)
 *@block index
 *struct erase_entry
 *erase_buffer size = count * sizeof(struct erase_entry_map)
 */
struct ssd_mark_erase_count_param
{
    ssd_u8 channel;
    ssd_u16 count;
    ssd_u8 *erase_buffer;
};

struct BlockId 
{
    uint64_t m_nLow;  ///< low 64 bits
    uint64_t m_nHigh; ///< high 64 bits
/*
    /// @brief operator==
    bool operator==(const BlockId& id) const {
        return m_nHigh == id.m_nHigh && m_nLow == id.m_nLow;
    }
    /// @brief operator!=
    bool operator!=(const BlockId& id) const {
        return m_nHigh != id.m_nHigh || m_nLow != id.m_nLow;
    }
    /// @brief operator<
    bool operator<(const BlockId& id) const {
        return m_nHigh < id.m_nHigh || (m_nHigh == id.m_nHigh && m_nLow < id.m_nLow);
    }
    /// @brief operator>
    bool operator>(const BlockId& id) const {
        return m_nHigh > id.m_nHigh || (m_nHigh == id.m_nHigh && m_nLow > id.m_nLow);
    }
    /// @brief operator<=
    bool operator<=(const BlockId& id) const {
        return m_nHigh < id.m_nHigh || (m_nHigh == id.m_nHigh && m_nLow <= id.m_nLow);
    }
    /// @brief operator>=
    bool operator>=(const BlockId& id) const {
        return m_nHigh > id.m_nHigh || (m_nHigh == id.m_nHigh && m_nLow >= id.m_nLow);
    }
    /// @brief operator=
    BlockId& operator=(const BlockId& id) {
        this->m_nHigh = id.m_nHigh;
        this->m_nLow = id.m_nLow;
        return (*this);
    }
    /// @brief operator<<
    friend ostream& operator<<(ostream& out, const BlockId& id) {
        char buf[33];
        id.ToHexString(buf);
        out << buf;
        return out;
    }
    /// @brief Default constructor
    BlockId() {
        m_nHigh = 0;
        m_nLow = 0;
    }
    /// @brief Constructor
    BlockId(uint64_t val) {
        m_nHigh = 0;
        m_nLow = val;
    }
    /// @brief ToHexString
    void ToHexString(char* pBuf) const {
        FormatUint64(m_nHigh, pBuf);
        FormatUint64(m_nLow, pBuf + 16);
        pBuf[32] = 0;
    }
    /// @brief FromHexString
    bool FromHexString(const char* pBuf) {
        m_nHigh = 0;
        m_nLow = 0;
        return GetUint64(m_nHigh, pBuf) && GetUint64(m_nLow, pBuf + 16);
    }
    /// @brief Get media type
    MediaType GetMediaType() const {
        return (MediaType)((m_nHigh & 8) >> 3);
    }
    /// @brief Set media type
    void SetMediaType(MediaType type) {
        m_nHigh = (m_nHigh & (((uint64_t)-1)-8)) | (type << 3);
    }
    /// @brief Get block type
    BlockType GetBlockType() const {
        return (m_nHigh & 7) == 0 ? BT_INDEX : BT_DATA;
    }
    /// @brief Get block serial type
    BlockSerialType GetBlockSerialType() const {
        return (BlockSerialType)(m_nHigh & 7);
    }
    /// @brief GetTableId
    uint32_t GetTableId() const {
        return (m_nHigh >> 34);
    }

    TabId GetGlobalTabId() const {
        return m_nHigh >> 4;
    }

    /// @brief Split Global TabId to TableId, local UnitId and local TabId.
    static void SplitGlobalTabId(const TabId gtabid, uint32_t& tbid, 
                          uint32_t& uid, uint32_t& tid) {
         tid = gtabid & 255;
         uid = (gtabid >> 8) & 4194303;
         tbid = (gtabid >> 30);
         
    }

    /// @brief Generate Global UnitId from tableid and unitid.
    static UnitId GenGlobalUnitId(const uint32_t nTableId, const uint32_t nUnitId) {
         return  ((uint64_t)nTableId <<22 | (uint64_t)nUnitId);
    }

    /// @brief GetUnitId
    UnitId GetGlobalUnitId() const {
        return (m_nHigh >> 12);
    }
    /// @brief GetUnitId
    uint32_t GetUnitId() const {
        return (m_nHigh >> 12) & 4194303; // 2^22 - 1
    }
    /// @brief GetTabId
    uint32_t GetTabId() const {
        return (m_nHigh >> 4) & 255; // 2^8 - 1
    }
    /// @brief GetSerialId
    uint64_t GetSerialId() const {
        return m_nLow;
    }
    */
};

/**************************************
 *      Macros
 **************************************/ 
#define IOCTL_IOC_MAGIC          0x86    
#define IOCTL_OPEN               _IO(IOCTL_IOC_MAGIC, 1)
#define IOCTL_CLOSE              _IO(IOCTL_IOC_MAGIC, 2)
#define IOCTL_FLUSH              _IO(IOCTL_IOC_MAGIC, 3)
#define IOCTL_DELETE             _IOW(IOCTL_IOC_MAGIC, 4, struct ssd_blockid)
#define IOCTL_WRITE              _IOW(IOCTL_IOC_MAGIC, 5, struct ssd_write_param)
#define IOCTL_READ               _IOW(IOCTL_IOC_MAGIC, 6, struct ssd_read_param)
#define IOCTL_READ_TABLE         _IOW(IOCTL_IOC_MAGIC, 7, struct ssd_table_param)
//#define IOCTL_WRITE_RETRY        _IOW(IOCTL_IOC_MAGIC, 8, struct ssd_write_retry_param)
#define IOCTL_READ_ID_TABLE      _IOW(IOCTL_IOC_MAGIC, 9, struct ssd_blockid *)
#define IOCTL_UNITTEST           _IO(IOCTL_IOC_MAGIC, 10)
#define IOCTL_SYS_INIT           _IO(IOCTL_IOC_MAGIC, 11)

#define IOCTL_REG_READ           _IOW(IOCTL_IOC_MAGIC, 12, struct ssd_regrd_param)
#define IOCTL_REG_WRITE          _IOW(IOCTL_IOC_MAGIC, 13, struct ssd_regwr_param)

#define IOCTL_MARK_BAD_BLOCK     _IOW(IOCTL_IOC_MAGIC, 14, struct ssd_mark_bad_param)
#define IOCTL_MARK_ERASE_COUNT   _IOW(IOCTL_IOC_MAGIC, 15, struct ssd_mark_erase_count_param)
#define IOCTL_READ_FLUSH       _IOW(IOCTL_IOC_MAGIC, 16, struct ssd_meta_version)

#define IOCTL_RECOVER_OPEN       _IO(IOCTL_IOC_MAGIC, 17)
#define IOCTL_RECOVER_DELETE     _IOW(IOCTL_IOC_MAGIC, 18, struct ssd_blockid)
#define IOCTL_WRITE_TABLE        _IOW(IOCTL_IOC_MAGIC, 19, struct ssd_table_param)
#define IOCTL_ERASE_BLOCK        _IOW(IOCTL_IOC_MAGIC, 20, struct ssd_block_param)

/* function return values */
 enum SSD_ERROR_TYPE{
    SSD_ERROR  = (-1),
    SSD_OK = 0,
    SSD_ERR_START_PAGE_ALIGN = 1,
    SSD_ERR_LEN_PAGE_ALIGN = 2,
    SSD_ERR_START_BLOCK_ALIGN = 3,
    SSD_ERR_LEN_BLOCK_ALIGN = 4,
    SSD_ERR_PARAM_OVERFLOW = 5,
    SSD_ERR_PARAM_WRLEN = 6,
    SSD_ERR_NULL = 7,
    SSD_ERR_NOMEM = 8,
    SSD_ERR_TIMEOUT = 9,
    SSD_ERR_NO_DEV = 10,
    SSD_ERR_NOT_SSD = 11,
    SSD_ERR_DMA_MAP = 12,
    SSD_ERR_DEL_TIMER = 13,
    SSD_ERR_COPY_FROM_USER = 14,
    SSD_ERR_COPY_TO_USER = 15,
    SSD_ERR_CHANNEL = 16, /* the channel status is disable */
    SSD_ERR_SSD = 17, /* the ssd status is disable */
    SSD_ERR_WAIT = 18, /* the operation is wait for complete */
    SSD_ERR_BAD_BLOCK = 19,
    SSD_ERR_NO_INVALID_BLOCKS = 20,
    SSD_ERR_TABLE_TYPE = 21,
    SSD_ERR_OEPN_DEV_FAILED = 22,
    SSD_ERR_NO_VALID_FD = 23,
    SSD_ERR_NOT_OPENED = 24,
    SSD_ERR_NOT_CLOSED = 25,
    SSD_ERR_ALLOC_BLOCK = 26,
    SSD_ID_NO_EXIST = 27,
    SSD_ID_ALREADY_EXIST = 28,
    SSD_ID_MAP_ERROR = 29,
    SSD_ERR_NO_VALID_RESERVE = 30,
    SSD_ERR_META_BLOCK_ALIGN = 31,
    ENOMETA = 32,
    EREADFLUSH = 33,
    EMETAUNVAL = 34,
    EIDUNVAL = 35,
    ETABLEDATA = 36,
    SSD_ERR_INVALID_PARAM = 37,
    SSD_ERR_NOT_BAD_BLOCK = 38,
};

/**************************************
 * functions declaration for ssd operation
 **************************************/
int SSD_open(int dev_num);
void SSD_close(int dev_num);
uint64_t ssd_reg_read(uint64_t addr);
int ssd_reg_write(uint64_t addr, uint64_t value);
int SSD_flush(int dev_num);
uint64_t SSD_util(int dev_num);
int SSD_delete(int dev_num, struct BlockId id);
int SSD_write(int dev_num, struct BlockId id, char * buffer, size_t count, off_t offset);
int SSD_read(int dev_num, struct BlockId id, char *buffer, size_t count, off_t offset);
int SSD_ftw(int dev_num, int (*fn) (const char *fpath, const struct stat *sb, int typeflag), int depth);
int SSD_open_newcard(int dev_num);
int SSD_close_force(int dev_num);
int SSD_mark_bad_block(int dev_num, int type, struct BlockId id, ssd_u16 block, ssd_u16 channel);
/*
 *param.erase_buffer = (ssd_u8 *)buffer;
 *buffer size = count * sizeof(struct erase_entry_map);
 */
int SSD_mark_erase_count(int dev_num, struct erase_entry_map *buffer, ssd_u16 count, ssd_u8 channel);

#endif
/*----------------------------------------------------------------------------*
REV #  DATE       BY    REVISION DESCRIPTION
-----  --------  -----  ------------------------------------------------------
0001   12/10/31  wangyong   Initial version.
0002   13/07/09  suijulei   Version 1-0-0-5
*----------------------------------------------------------------------------*/

