/*
 *  h/ssd_drv.h
 *
 *   define types of ssd driver
 *
 *  Copyright (C) 2001 baidu.com
 *
 *  2011-10-24  create by wangyong<wangyong03@baidu.com>
 *  2012-11-20  modified by suijulei<suijulei01@baidu.com>
 *  2013-10-14  modified by zhangwei<zhangwei29@baidu.com>
*/

#ifndef SSD_DRV_INCLUDED
#define SSD_DRV_INCLUDED

/**************************************
 * Header Include
 **************************************/
#include <linux/genhd.h>
#include <linux/pci.h>
#include <linux/module.h>
#include <linux/semaphore.h>
#include <linux/completion.h>
#include <linux/spinlock.h>
#include <linux/blkdev.h>
#include <linux/bitmap.h>
#include <asm/atomic.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <linux/err.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/file.h>
#include <linux/stat.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/proc_fs.h>
#include <linux/bug.h>
#include "ssd_type.h"

/**************************************
 *      Constants 
 **************************************/
/* use for unit_test */
//#define UNIT_TEST  
#define TEST_OFF 0
#define TEST_ON  1
int reg_read_unittest = TEST_OFF;
uint64_t reg_read_value0 = 0;
uint64_t reg_read_value1 = 0;
uint64_t reg_read_value2 = 0;
	
/* use for debug */

#define SSD_DRIVER_VERSION 0x0
#define SSD_MODULE_VERSION "2-1-0-0"


/* the range of device minor number */
#define SSD_MINORS 1     

/* the bar number of configure space */
#define SSD_BAR  0 

#define SSD_DEVICE_NAME  "ssd_Baidu"
#define SSD_DRIVER_NAME  "ssd_driver"

/* the pci vendor id of baidu */
#define VERDOR_ID_BAIDU    0x10EE

/* the pci device id of baidu SSD device */
#define SSD_DEEPOCEAN_DEVICE_ID    0x0007

/* buffer size of ssd read or write for dma operation */
#define SSD_BUFFER_SIZE   (10*1024*1024)


#define MAX_DEVNAME 32

/* status type of ssd or channels */
#define STATUS_ENABLE 0
#define STATUS_DISABLE 1

#define SSD_CHANNEL_TIMEOUT (5 * HZ)
#define SSD_INIT_TIMEOUT    (2 * HZ)

/*
 * SSD_S6_LEN is the len of one s6 chip in bar0 region
 * the address of registers
 *  0k ~  4K-1    S6-0 registers
 *  4k ~  8k-1    S6-1 registers
 *  8k ~ 12k-1    S6-2 registers
 * 12k ~ 16k-1    S6-3 registers
 * 32k ~          ssd configure registers      
 */
 
#define SSD_S6_BASE  0x0LL
#define SSD_CONFIG_BASE 0x8000LL
#define SSD_S6_LEN   4096   
#define SSD_S6_CHANNEL_LEN 256

/* the offset of each register in bar0 start from SSD_CONFIG_BASE */
#define SSD_IRQ_PENDING     0x8018ULL
#define SSD_IRQ_S6          0x8020ULL
#define SSD_S6_IRQ0         0x8020ULL
#define SSD_S6_IRQ1         0x8028ULL
#define SSD_S6_IRQ2         0x8030ULL
#define SSD_S6_IRQ3         0x8038ULL

#define SSD_STATUS          0x8000ULL
#define SSD_CHANNEL_STATUS  0x8008ULL
#define SSD_RAM_SIZE        0x8010ULL
#define SSD_BAD_BLOCK       0x8018ULL


/* the types of channel operation */
#define SSD_FLAG_READ                   0x0
#define SSD_FLAG_WRITE                  0x1
#define SSD_FLAG_ERASE                  0x2
#define SSD_FLAG_READTABLES_FROM_FLASH  0x4   //Read the persistent information(bad block tables and address map table) from flash to host
#define SSD_FLAG_WRITETABLE_TO_FLASH    0x5   //Write the persistent information from host to flash
#define SSD_FLAG_ERASETABLES            0x6   //Earse the persistent information in flash
#define SSD_FLAG_WRITE_ADDR_TABLE       0x3   //Write address mapping table from host to fpga ram
#define SSD_FLAG_READ_ADDR_TABLE        0x8   //Read address mapping table from fpga ram to host
#define SSD_FLAG_WRITE_ERASE_TABLE      0x7   //Write bad block table form host ram to fpga ram
#define SSD_FLAG_READ_ERASE_TABLE       0x9   //Read bad block table from fpga ram to host ram
#define SSD_FLAG_READRAM                0xa   
#define SSD_FLAG_BACKUP                 0xb  
#define SSD_FALG_INIT                   0xc
#define SSD_FLAG_READOOB                0xd  


/* the pages each table */
#define SSD_PAGES_PER_TABLE  2   

/* the planes count to store one type table */
#define SSD_MAX_TABELS (SSD_PAGES_PER_BLOCK / SSD_PAGES_PER_TABLE)    //128

#define SSD_MAX_TRY_COUNT  4

/* the number of physical blocks  for each channel */
#define SSD_MAX_PHYBLOCKS_PER_CHANNEL   8192

/* the number of logic blocks for each channel */
#define SSD_MAX_VIRBLOCKS_PER_CHANNEL   8000

#define PHY_PLANE01_BLOCK_VALID_START 0x0A
#define PLANE_TABLE_RESERVE_COUNT PHY_PLANE01_BLOCK_VALID_START

#define PLANE01_MAX_TABLE_INDEX PLANE_TABLE_RESERVE_COUNT

/* each plane reserved 5 blocks for storing tables */
#define PLANE_RESERVE_BLOCKS    0x5
/* blocks number(2048) for each plane */
#define BLOCKS_PER_PLANE        0x800
#define PLANE_VALID_BLOCKS      (BLOCKS_PER_PLANE - PLANE_RESERVE_BLOCKS)

#define PHY_PLANE23_BLOCK_VALID_START 0x100A
//#define PLANE23_MAX_TABLE_INDEX PHY_PLANE23_BLOCK_VALID_START


#define ADDR_MASK          0x1fff


#define PLANE0_TABLE_START     0x0
#define PLANE1_TABLE_START     0x1
#define PLANE2_TABLE_START     0x1000
#define PLANE3_TABLE_START     0x1001

#define SSD_MAX_PLANES   4

/********************************************************/
#define TABLE_DUPLICATE             2

#define SSD_MAX_FLUSH_META_CHANNEL  2
#define SSD_MAX_FLUSH_TABLE_CHANNEL 42
#define SSD_MAX_CHANNEL_EACH_TABLE  (SSD_MAX_FLUSH_TABLE_CHANNEL / TABLE_DUPLICATE)


#define SSD_MAX_FLUSH_BLOCK         5
#define SSD_MAX_FLUSH_SIZE          (SSD_MAX_FLUSH_BLOCK * SSD_BLOCK_SIZE)

#define SSD_MAX_FLUSH_TABLE_RETRY   5
#define SSD_MAX_FLUSH_META_RETRY    3

#define SSD_MIN_META_DUPLICATE      3

#define RESERVE_BLOCKS_PER_CHANNEL  20
#define RESERVE_BLOCKS_TABLE        (SSD_MAX_FLUSH_TABLE_CHANNEL * RESERVE_BLOCKS_PER_CHANNEL)
#define RESERVE_BLOCKS_META         (SSD_MAX_FLUSH_META_CHANNEL * RESERVE_BLOCKS_PER_CHANNEL)
#define RESERVE_BLOCKS_TOTAL        (RESERVE_BLOCKS_TABLE + RESERVE_BLOCKS_META)

#define TABLE_BITMAP_SIZE           (RESERVE_BLOCKS_TABLE / TABLE_DUPLICATE)
#define META_BITMAP_SIZE            (RESERVE_BLOCKS_META / SSD_MAX_PLANES)

#define RESERVE_BLOCKS_EACH_TABLE   TABLE_BITMAP_SIZE

#define SSD_MAX_VIRBLOCKS           (SSD_MAX_CHANNEL * SSD_MAX_VIRBLOCKS_PER_CHANNEL)
#define SSD_MAX_ID_NUM              SSD_MAX_VIRBLOCKS
#define SSD_MAX_ID_NUM_PER_CHANNEL  SSD_MAX_VIRBLOCKS_PER_CHANNEL

#define ADDR_TABLE_PAGES            2
#define SSD_TABLE_SIZE              (ADDR_TABLE_PAGES * SSD_PAGE_SIZE)
#define SSD_TABLE_TYPE_NUM          2
#define SSD_BLOCK_SHIFT             21

#define FLUSH_MAGIC_NORMAL          0x5A9B
#define FLUSH_MAGIC_CLOSE           0xC43A

#define RESERVE_BLOCKS_META_IN_PLANE    (RESERVE_BLOCKS_META / SSD_MAX_PLANES)

#define BT_INDEX_BLOCK_NUM              1
#define BT_DATA_BLOCK_NUM               5
#define BT_INDEX_SIZE                   (BT_INDEX_BLOCK_NUM * SSD_BLOCK_SIZE)
#define BT_DATA_SIZE                    (BT_DATA_BLOCK_NUM * SSD_BLOCK_SIZE)

#define SSD_CLOSED                      0
#define SSD_OPENED                      1

#define SSD_MAX_WRITE_RETRY				5


/**************************************
 *      Macros
 **************************************/ 
#define CHANNEL_STATUS(sdev, i)  ((sdev)->channel_ctl[i].status)

#define SEND_DATA_FLAG_MASK  0x1fLL                /* Mask bits 0-4 */
#define SEND_DATA_START_MASK 0x1fffffffeLL        /* Mask bits 5-36 */
#define SEND_DATA_LEN_MASK   0xffe000000000LL /* Mask bits 37-47 */

static inline ssd_u64 cmd_value(ssd_u32 start, ssd_u32 len, ssd_u8 flag)
{
    return ((ssd_u64)flag & SEND_DATA_FLAG_MASK) | 
           ((((ssd_u64)start) << 5) & SEND_DATA_START_MASK)|
           ((((ssd_u64)len) << 37) & SEND_DATA_LEN_MASK);
}


/**************************************
 *      Data types
 **************************************/ 

/* the control struct of channel operation */
typedef struct channel_op_control {
    ssd_status status;            
	ssd_u32 redzone0;
    ssd_ret errors;              
	volatile ssd_ret error_ram;
	ssd_u32 redzone1;
    ssd_u8 channel;         
    ssd_u64 op_counts;         
    struct semaphore sema;    
	ssd_u32 redzone2;
    struct completion *waiting;  
	ssd_u32 redzone3;
    struct timer_list  timeout;
	ssd_u32 redzone4;
    struct ssd_device *sdev;  
    int plane_cur[SSD_MAX_PLANES];
}channel_op_control_t;

typedef struct id_info {
    ssd_u32 channel  : 6;
    ssd_u32 block    :13;
    ssd_u32 len      : 1;
} id_info_t;

/*for ssd device*/
struct ssd_hash_node
{
    struct ssd_blockid id;
    ssd_u32 channel  : 6;
    ssd_u32 block    :13;
    ssd_u32 len      : 1;
    struct ssd_hash_node *next;
} ssd_hash_node_t;

#define SSD_HASH_SIZE 391
#define ATOMIC_INIT(i)  ( (atomic_t) { (i) } )

struct ssd_hash_head
{
    atomic_t count; 
    spinlock_t hash_lock;
    struct ssd_hash_node *first;
}__attribute__((packed)) hash_node_head_t;

struct channel_hash_list
{
    struct ssd_hash_head head[SSD_HASH_SIZE];
};

/***********************************************/

struct flush_id_info
{
    struct ssd_blockid id;
    ssd_u32 channel  : 6;   /* channel = 0...43 */
    ssd_u32 block    :13;   /* block = 0...19   */
    ssd_u32 len      : 1;   /* 0=2M, 1=10M      */
    ssd_u32 crc32;
};

struct meta_position
{
    ssd_u8 page;            /* page = 0 ... 255  */
    ssd_u16 block_pos;       /* persition = 0, 4, 8, 12, 16; 20, 24, 28, 32, 36 */
};

struct ssd_metadata
{
    ssd_u16 metadata_magic;
    ssd_u16 block_num;
    ssd_u32 last_block_used_bytes;
    ssd_u32 id_num;
    ssd_u64 version;
    time_t timestamp;
    ssd_u32 metadata_crc32;
    /* table_block_pos = (20block*2channel) ~ (20block*44channel - 1) */
    ssd_u16 table_block_pos[SSD_MAX_FLUSH_BLOCK * TABLE_DUPLICATE]; /*2 duplicate*/ 
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

/*just for setting erase count*/
struct erase_entry_map
{
    ssd_u16 block;
    struct erase_entry ee;
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

enum FLUSH_TYPE {
    FLUSH_NORMAL_TYPE = 0,
    FLUSH_CLOSE_TYPE = 1,
};


enum block_type{
    IBLOCK = 0, /*index block*/
    DBLOCK,	/*data block*/

    ALLBLOCKS
};

int BLOCKSIZE[ALLBLOCKS] = {
    BT_INDEX_BLOCK_NUM,  /*1 block = 2M*/
    BT_DATA_BLOCK_NUM   /*5 blocks = 10M*/
};

#define CURRENT_AREA_THROTTLE (SSD_MAX_VIRBLOCKS_PER_CHANNEL * 9 / 10)
#define FREE_AREA_RECLAIM_THROTTLE (SSD_MAX_VIRBLOCKS_PER_CHANNEL * 1 / 100)
/*-----------------------
 *| 200M |     1G     | other |
 *-----------------------
*/
#define PRE_ALLOC_BLOCKS (SSD_MAX_VIRBLOCKS_PER_CHANNEL / 80) /*100 * 2M + 100 * 10M = 1200M*/
#define PRE_ALLOC_START 0

struct free_area_bitmap {
    spinlock_t bitmap_lock;
    ssd_u16 type;
    DECLARE_BITMAP(area_map, SSD_MAX_VIRBLOCKS_PER_CHANNEL);
};

/*fab[0] -- IBLOCK
*  fab[1] -- DBLOCK
*/
struct free_area_list {
    struct  free_area_bitmap fab[2];
};

typedef struct ssd_device {
    int major;                      
    ssd_u8 status; 
    ssd_u8 channel_num;   
    char name[MAX_DEVNAME];
    
    void __iomem *conf_regs;      /* Virtual address of bar0 ram space */
    void __iomem* conf_channel_addr[SSD_MAX_CHANNEL];
    void __iomem* conf_channel_cmd[SSD_MAX_CHANNEL];

    struct gendisk *gdisk[SSD_MAX_CHANNEL];           
    struct module *owner;
    struct pci_dev *pdev;
    spinlock_t queue_lock;  
    struct channel_hash_list ch_hash[SSD_MAX_CHANNEL];
    struct kmem_cache *hash_node_slab;
  /*
    *--------------------------------
    *|    for free area        |   reserver    |
    *--------------------------------
    *                           current
    */
    struct free_area_list area[SSD_MAX_CHANNEL]; /*for logic block allocation*/
    ssd_s16 free_blocks[SSD_MAX_CHANNEL]; /*all free blocks*/
    ssd_u16 current_free_area[SSD_MAX_CHANNEL]; /*consecutive blocks on SSD, other than blocks in free area*/

    struct request_queue *queue;

    struct ssd_info sinfo;         /* the information of ssd */  

    channel_op_control_t channel_ctl[SSD_MAX_CHANNEL];

    /* kernel buffer for read/write request */
    ssd_u8 *kbuftable_cpu[SSD_MAX_CHANNEL];
    dma_addr_t kbuftable_dma[SSD_MAX_CHANNEL];
    
    /* for flush */
    struct ssd_metadata meta;
    struct meta_position meta_pos;
    
    ssd_u8 *kbuf_flush;

    ssd_u64 local_bus_error; /* check for local bus error */
    
}ssd_device_t;


/**************************************
 * local Variables declaration
 **************************************/
 /*
static struct proc_dir_entry* proc_ssd_dir = NULL;
static struct proc_dir_entry* proc_ssd_major_entry = NULL;
static struct proc_dir_entry* proc_ssd_channels_entry = NULL;
*/
/**************************************
 * Global Variables declaration
 **************************************/
static int ssd_major = 0;
static struct ssd_device *ssd_dev = NULL;

/*  alloc memory buffer when kernel start before mm_init function */
extern char *ssd_buffer_start;

#endif /* SSD_DRV_INCLUDED */
