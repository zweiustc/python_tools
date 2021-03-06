/*
 *  drv/ssd_drv.c
 *
 *  ssd driver module
 *
 *  Copyright (C) 2001 baidu.com
 *
 *  2011-10-24  create by wangyong<wangyong03@baidu.com>
 *  2012-11-20 modified by suijulei<suijulei01@baidu.com>
*/


#include "ssd_drv.h"

static inline ssd_u64 ssd_reg64_read(void *addr);
static inline void ssd_reg64_write(void *addr, ssd_u64 val);
static struct file * ssd_log_open(const char *filename, int flag, int mode);
static void ssd_log_close(void);
static int ssd_log_write(struct file *fp, char *buffer, int len);
static void ssd_log(int log_level, const char *fmt, ...);
static int ssd_block_open(struct block_device *bdev, fmode_t mode);
static int ssd_block_release(struct gendisk *gdisk, fmode_t mode);
static int ram_table_read(struct ssd_device *sdev, struct ssd_table_param *param);
static int ssd_table_read(struct ssd_device *sdev, struct ssd_table_param *param);
static ssd_ret ssd_delete_timer(channel_op_control_t *ch_ctl);
static irqreturn_t ssd_interrupt(int irq, void *sdev_instance);
static int ssd_set_conf(struct ssd_device *sdev);
static void ssd_unset_conf(struct ssd_device *sdev);
static int ssd_enable_msi(struct ssd_device *sdev);
static void ssd_disable_msi(struct ssd_device *sdev);
static int  __devinit ssd_probe(struct pci_dev *pdev, const struct pci_device_id *ent);
static void __devexit ssd_remove (struct pci_dev *pdev);
static int ssd_create_slab(struct ssd_device *sdev);
static void ssd_init_hash(struct ssd_device *sdev);
static void ssd_destory_slab(struct ssd_device *sdev);
static ssd_u16 ssd_init_channel_bitmap(struct free_area_list *area, ssd_u16 start, ssd_u16 pre_alloc);
static int ssd_reload_bitmap(struct ssd_device *sdev);
static int bitmap_split(struct free_area_bitmap *from, struct free_area_bitmap *to);
static int bitmap_merge(struct free_area_bitmap *from, struct free_area_bitmap *to);
static int ssd_remap_block(struct ssd_device *sdev, ssd_u8 channel);
static int ssd_alloc_logic_block(struct ssd_device *sdev, ssd_u8 channel, ssd_u16 type);
static int ssd_sys_open(struct ssd_device *sdev);
static int ssd_sys_close(struct ssd_device *sdev);
static void ssd_timeout(channel_op_control_t *ch_ctl);
static void ssd_add_timer(channel_op_control_t *ch_ctl, unsigned long timeout, 
                               void (*func)(channel_op_control_t *));
static int channel_op(struct ssd_device *sdev, ssd_u8 channel, ssd_u64 cmd);
static inline int id2channel(struct ssd_blockid *id);
static inline int id_hash(struct ssd_blockid *id);
static int set_id_info(struct ssd_device *sdev, struct ssd_blockid *id, id_info_t *info);
static int get_id_info(struct ssd_device *sdev, struct ssd_blockid *id, id_info_t *info);
static int del_id_info(struct ssd_device *sdev, struct ssd_blockid *id, id_info_t *info);
static int check_id_info(id_info_t *id_info);
//static int ssd_id_write_retry(struct ssd_device *sdev, struct ssd_write_retry_param * wrparam);
static int ssd_id_write(struct ssd_device *sdev, struct ssd_write_param *wparam);
static int ssd_id_read(struct ssd_device *sdev, struct ssd_read_param *rparam);
static void delete_logic_block(struct ssd_device *sdev, id_info_t *info);
static int ssd_id_delete(struct ssd_device *sdev, struct ssd_blockid *id);
static int copy_ram_table(struct ssd_device *sdev, ssd_u8 channel, char *buffer);
static inline void copy_hash_node_to_flush_id_info(struct ssd_hash_node *node, struct flush_id_info *sp);
static ssd_u32 copy_id_hash(struct ssd_device *sdev, struct flush_id_info *buffer);
static inline ssd_s32 find_new_table_pos(struct ssd_device *sdev, ssd_u16 cur_pos);
static inline ssd_s32 calc_phyblock(ssd_u16 block_pos);
static inline int write_one_reserve_block(struct ssd_device *sdev, ssd_u16 block_pos, char* buffer);
static inline int read_one_reserve_block(struct ssd_device *sdev, ssd_u16 block_pos);
static int erase_one_reserve_block(struct ssd_device *sdev, ssd_u16 block_pos);
static int write_tabledata_block_duplicate(struct ssd_device *sdev, ssd_u16 cur_pos, char *buffer);
static inline void update_meta(struct ssd_device *sdev, ssd_u16 block_num, ssd_u16 last_block_used_bytes, int flush_type, ssd_u32 id_num);
static inline void set_table_pos_in_meta(struct ssd_device *sdev, ssd_u16 block_num, ssd_u16 cur_pos);
static ssd_s32 retry_write_tabledata_block(struct ssd_device *sdev, ssd_u16 block_num, ssd_u16 last_pos, char *buffer);
static int flush_tabledata(struct ssd_device *sdev, char *buffer, ssd_u32 id_num, int flush_type);
static int write_one_reserve_page(struct ssd_device *sdev, ssd_u16 block_pos, ssd_u8 page);
static int read_one_reserve_page(struct ssd_device *sdev, ssd_u16 block_pos, ssd_u8 page);
static int write_meta_page(struct ssd_device *sdev, int plane_num);
static int write_metadata_page_duplicate(struct ssd_device *sdev);
static inline ssd_s32 find_new_meta_block(struct ssd_device *sdev, ssd_u16 cur_pos);
static inline int find_new_meta_pos(struct ssd_device *sdev, int last_success);
static int flush_metadata(struct ssd_device *sdev);
static int ssd_sys_flush(struct ssd_device *sdev, int flush_type);
static int ssd_read_idtable(struct ssd_device *sdev, struct ssd_readid_param *ridparam);  
static inline int metadata_check_val(struct ssd_metadata *metadata);
static inline int ssd_read_metadata(struct ssd_device *sdev, ssd_u16 meta_block, ssd_u8 meta_page);
static inline int ssd_read_tabledata(struct ssd_device *sdev, ssd_u16 tabledata_block, ssd_u8 page);
static int search_block_metadata(struct ssd_device *sdev);
static int search_page_metadata(struct ssd_device *sdev);
static int ssd_search_metadata(struct ssd_device *sdev);
static int ssd_read_flush_tabledata(struct ssd_device *sdev, char *buffer);
static int ssd_read_flush(struct ssd_device *sdev, struct ssd_meta_version *version);
static int ssd_id_val(struct flush_id_info *id, struct addr_entry *addr, struct erase_entry *erase);
static int ssd_rebuild_hash(struct ssd_device *sdev, char *buffer);
static inline int ssd_mark_bad_block(struct ssd_device *sdev, struct ssd_mark_bad_param *mbparam);
static inline int ssd_mark_erase_count(struct ssd_device *sdev, struct ssd_mark_erase_count_param *mecparam);
static inline void ssd_mark_erase_table(struct ssd_hash_node *node, struct addr_entry *addr, struct erase_entry *erase);
static int ssd_rebuild_erasetable(struct ssd_device *sdev, char *buffer);
static inline int ssd_write_tables_to_ram(struct ssd_device *sdev, ssd_u8 channel, ssd_u8 flag);
static inline int ssd_erase_phyblock(struct ssd_device *sdev, ssd_u8 channel, ssd_u16 block);
static int ssd_erase_unused_block(struct ssd_device *sdev, ssd_u8 channel, struct erase_entry *buf);
static int ssd_updata_tables(struct ssd_device *sdev, char *buffer, int normal_uninstall);
static int ssd_sys_init(struct ssd_device *sdev);
static int ssd_sys_open_newcard(struct ssd_device *sdev);
static int ssd_sys_init_newcard(struct ssd_device *sdev);
static int ssd_sys_init_newcard_in_channel(struct ssd_device *sdev, ssd_u8 channel);
static int ssd_erase_table_init(struct ssd_device *sdev, ssd_u8 channel);
static void create_erase_table(struct ssd_device *sdev, ssd_u8 channel);
static int ssd_addr_table_init(struct ssd_device *sdev, ssd_u8 channel);
static void create_addrmap_table(struct ssd_device *sdev, ssd_u8 channel);
static inline void ssd_init_meta(struct ssd_device *sdev);
static int ssd_block_ioctl(struct block_device *bdev, fmode_t mode, unsigned int cmd, unsigned long arg);
static void ssd_block_request(struct request_queue *q);
static int ssd_add_device(struct ssd_device *sdev);
static int __init ssd_init_module(void);
static void ssd_del_device(struct ssd_device * sdev);
static void __exit ssd_remove_module(void);
static int create_ssd_proc(void);
static int del_ssd_proc(void);
static int proc_speed_read(char *page, char **start, off_t off, int count, int *eof, void *data);

static inline ssd_u64 ssd_reg64_read(void *addr)
{
    ssd_u64 val;

    val = readq(addr);
    smp_rmb();
    
    return val;
}

static inline void ssd_reg64_write(void *addr, ssd_u64 val)
{
    writeq(val, addr);
    smp_wmb();
}

static struct file *g_ssd_log_fp = NULL;
static char* LOG_PATH = "/home/disk1/ssd_driver.log";
int g_opened_flag = SSD_CLOSED;

static unsigned int read_count[SSD_MAX_CHANNEL] ={0};
static unsigned int write_count[SSD_MAX_CHANNEL] = {0};
static unsigned int delete_count[SSD_MAX_CHANNEL] = {0};
static unsigned int read_count_buf[SSD_MAX_CHANNEL] ={0};
static unsigned int write_count_buf[SSD_MAX_CHANNEL] = {0};
static unsigned int delete_count_buf[SSD_MAX_CHANNEL] = {0};

static int free_blocks[SSD_MAX_CHANNEL] = {0};
static int free_blocks_buf[SSD_MAX_CHANNEL] = {0};

static struct proc_dir_entry *proc_ssd_dir;
static struct proc_dir_entry *proc_ssd_speed_entry;

struct semaphore flush_sema;

static struct file * ssd_log_open(const char *filename, int flag, int mode)
{
    struct file *fp;
    if(g_ssd_log_fp)
    return g_ssd_log_fp;
    
    fp = filp_open(filename, flag, mode);

    if (IS_ERR(fp))
        return NULL;
    else
        return fp;
}

static void ssd_log_close()
{
    struct file *fp = g_ssd_log_fp;

    g_ssd_log_fp = NULL;
    (void)schedule_timeout(3000); // for synchronization
    
    if (fp)
        filp_close(fp, NULL);

}

static int ssd_log_write(struct file *fp, char *buffer, int len)
{
    mm_segment_t fs; 
    int write_len;

    if ((buffer == NULL) || (len == 0))
        return -ENOENT;

    /* g_ssd_log_fp == NULL, the file to write log is closed */
    if ((fp == NULL) || (fp->f_op->write == NULL))
        return 0;

    if(((fp->f_flags & O_ACCMODE) & (O_RDWR | O_WRONLY)) == 0)
        return -EACCES;

    fs = get_fs();

    set_fs(get_ds());
    write_len = fp->f_op->write(fp, buffer, len, &fp->f_pos);
    set_fs(fs);

    return write_len;
}

enum {
    SSD_LOG_EMG     = 0,               /* emergency conditions */
    SSD_LOG_ERR     = 1,               /* error conditions */
    SSD_LOG_WARNING = 2,               /* action must be taken immediately */
    SSD_LOG_INFO    = 3,               /* informational */
    SSD_LOG_TRACE   = 4,               /* debug-level messages */
};

static char *log_level_string[] =
{
    "EMG",
    "ERR",
    "WARNING",
    "INFO",
    "TRACE"
};

/* default is STL_LOG_WARNING */
static int g_ssd_log_level = SSD_LOG_WARNING;//5;//SSD_LOG_INFO;
#define SSD_LOG_MAX_LINE  128 

// need to be tested for multi threads
static void ssd_log(int log_level, const char *fmt, ...)
{
    int ret;
    va_list args;
    char buffer[SSD_LOG_MAX_LINE];
    struct timex txc;
    struct tm result;
    int cnt;

    if (log_level > g_ssd_log_level)
        return;

    if (g_ssd_log_fp == NULL) {
        va_start(args, fmt);
        (void)vprintk(fmt, args);
        va_end(args);
    } else {
        memset(buffer, 0, SSD_LOG_MAX_LINE);
        do_gettimeofday(&(txc.time));
        time_to_tm(txc.time.tv_sec, 8 * 3600, &result);
        cnt = snprintf(buffer, sizeof(buffer),
                        "%ld-%d-%d %d:%d:%d [%s] \t",
                        result.tm_year + 1900,
                        result.tm_mon + 1, 
                        result.tm_mday,
                        result.tm_hour,
                        result.tm_min,
                        result.tm_sec,
                        log_level_string[log_level]);
        
        va_start(args, fmt);
        (void)vsnprintf(buffer + cnt, SSD_LOG_MAX_LINE - cnt, fmt, args);
        va_end(args);
        ret = ssd_log_write(g_ssd_log_fp, buffer, strlen(buffer));
        if (unlikely(ret < 0))
            printk(KERN_WARNING "SSD: write log failed\n");
    }

    return;
}

static int ssd_block_open(struct block_device *bdev, fmode_t mode)
{
    struct ssd_device *dev = bdev->bd_disk->private_data; 

    if (!try_module_get(dev->owner)) {
        printk("ssd_block_open failed\n");
        return -ENODEV;
    }

    return SSD_OK;
}

static int ssd_block_release(struct gendisk *gdisk, fmode_t mode)

{
    struct ssd_device *sdev = gdisk->private_data;
    
    module_put(sdev->owner);
    return SSD_OK;
}

/* read table from ddr ram on ssd */
static int ram_table_read(struct ssd_device *sdev, struct ssd_table_param *param)
{
    /* start addr is not used in ram table read */
    ssd_u8 flag;
    ssd_u64 cmd;

    if (param->type == TABLE_ERASE_TYPE)
        flag = SSD_FLAG_READ_ERASE_TABLE;
    else
        flag = SSD_FLAG_READ_ADDR_TABLE;
    cmd = cmd_value(0, ADDR_TABLE_PAGES, flag);
    
    return channel_op(sdev, param->channel, cmd); 
}

static int ssd_table_read(struct ssd_device *sdev, struct ssd_table_param *param)
{
    int ret;
    ssd_u8 channel = param->channel;
    struct semaphore *sema = &(sdev->channel_ctl[channel].sema);

    down(sema);

    ret = ram_table_read(sdev, param);
    if (ret != SSD_OK) {  
        ssd_log(SSD_LOG_ERR, "ssd_table_read: ram_table_read failed, channel = %u type = %u ,ret[%d] (line %d)\n", param->channel, param->type, ret, __LINE__);
        goto out;
    }
    
    if (copy_to_user(param->usr_buf, sdev->kbuftable_cpu[channel], SSD_TABLE_SIZE)) {
        ssd_log(SSD_LOG_ERR, "ssd_table_read: copy to user failed (line %d)\n", __LINE__);
        ret = -SSD_ERR_COPY_TO_USER;
        goto out;
    }

out:
    up(sema);
    
    return ret;
}

static ssd_ret ssd_delete_timer(channel_op_control_t *ch_ctl)
{
    ssd_ret ret = -SSD_ERR_DEL_TIMER;
    struct timer_list *timer = &(ch_ctl->timeout); 
    
    if (timer->function) {
        if ((del_timer(timer)) == 1)
            ret = SSD_OK;
        timer->data = (unsigned long)NULL;
        timer->function = NULL;
    }
    
    return ret;
}

static irqreturn_t ssd_interrupt(int irq, void *sdev_instance)
{
    struct ssd_device * sdev = (struct ssd_device *)sdev_instance;
    channel_op_control_t *ch_ctl;
    ssd_u64 pending;  /* the pending flag of all interrupts status */
    ssd_u64 s6_status;  /* the status of each s6 */
    ssd_s16 s6_pending; /* the s6 error value */
    int i, j;

    /* 0~48bits for channel operation interrupt, 49~64bits for other opeartion interrupt */
    pending = ssd_reg64_read(sdev->conf_regs + SSD_IRQ_PENDING);    
    //printk("interrupt: pending is 0x%xll\n", pending);
   
    for (i = 0; i < SSD_S6_NUMS; i++) {
        s6_pending = (ssd_s16)(pending >> (i * 16)) & 0xffff;        
        if (likely(s6_pending > 0)) {
            /* have interrupt and the operaton is success */  
            for (j = 0; j < SSD_CHANNELS_IN_S6; j++) {
                s6_pending <<= 1;
                if (s6_pending < 0) {                   
                    ch_ctl = &(sdev->channel_ctl[j + i * 11]); 
                    
                    if(-100 == ch_ctl->error_ram) {
                        ch_ctl->error_ram = 0;
                        continue;
                    }

                    if (unlikely(ch_ctl->errors == -SSD_ERR_TIMEOUT)) 
                        continue;
                    ch_ctl->errors = ssd_delete_timer(ch_ctl);
                    if ((ch_ctl->errors == SSD_OK) && (ch_ctl->waiting)) {                        
                        complete(ch_ctl->waiting);
                        ch_ctl->waiting = NULL;
                        continue;
                    }
                }
                else if (s6_pending > 0)
                    continue;
                else 
                    break;
            }
        }
        else if (s6_pending < 0) {
            /* have interrupt and the operation may be failed, so read the error value */  
            s6_status = ssd_reg64_read(sdev->conf_regs + SSD_IRQ_S6 + i * 8);
            for (j = 0; (j < SSD_CHANNELS_IN_S6) && s6_pending; j++) {
                s6_pending <<= 1;
                /* check each channel result */
                if (s6_pending < 0) {
                    ch_ctl = &(sdev->channel_ctl[j + i * 11]);
                    
                    if(-100 == ch_ctl->error_ram) {
                        ch_ctl->error_ram = 0;
                        continue;
                    }

                    if (unlikely(ch_ctl->errors == -SSD_ERR_TIMEOUT))
                        continue;
                    (void)ssd_delete_timer(ch_ctl);
                    /* each channel have 5 bits in s6_stauts */
                    ch_ctl->errors = (ssd_ret)(s6_status & 0x1f);
                    /*  0~6bits used for software error, other used for hardware error */
                    ch_ctl->errors <<= 7;
                    
                    if (ch_ctl->waiting) { 
                        complete(ch_ctl->waiting);
                        ch_ctl->waiting = NULL;
                    }
                }
                s6_status >>= 5; 
            }
        }
    }

    return IRQ_RETVAL(IRQ_HANDLED);
}

static int ssd_set_conf(struct ssd_device *sdev)
{
    int i;   
    channel_op_control_t *pch_ctl;

    if (!ssd_buffer_start) {
        printk(KERN_ERR "SSD:alloc memory failed when kernel init.\n");
        return -SSD_ERR_NOMEM;
    }

    /*added at 2012.11.29*/
    sdev->status = STATUS_ENABLE;
    sdev->channel_num = SSD_MAX_CHANNEL;
    sdev->major = ssd_major;
    
    //add at 2012.12.3
    ssd_init_meta(sdev);
        
    for (i = 0; i < SSD_MAX_CHANNEL; i++) {
        /* allocate kernel buffer for channels read or write */
        sdev->kbuftable_cpu[i] = ssd_buffer_start + SSD_BUFFER_SIZE * i;
        sdev->kbuftable_dma[i] = pci_map_single(sdev->pdev, 
                 sdev->kbuftable_cpu[i], SSD_BUFFER_SIZE, DMA_BIDIRECTIONAL);
        if (dma_mapping_error(&(sdev->pdev->dev), sdev->kbuftable_dma[i])) {
            printk(KERN_ERR "SSD: unable to map DMA buffer.\n");
            sdev->kbuftable_cpu[i] = NULL;
            break;
        }
    }

    if (i < SSD_MAX_CHANNEL) {
        while (i--) {
            if (sdev->kbuftable_cpu[i] != NULL) {
                pci_unmap_single(sdev->pdev, sdev->kbuftable_dma[i], SSD_BUFFER_SIZE, DMA_BIDIRECTIONAL);
                sdev->kbuftable_cpu[i] = NULL;
            }
        }
        return -SSD_ERR_NOMEM;
    }  

    //modified at 2012.11.28
    sdev->kbuf_flush = ssd_buffer_start + SSD_BUFFER_SIZE * SSD_MAX_CHANNEL;
    
    /* set the channel addr and command register addr */
    for (i = 0; i < SSD_MAX_CHANNEL; i++) {
        sdev->conf_channel_addr[i] = sdev->conf_regs + SSD_S6_BASE + 
                    (i / SSD_CHANNELS_IN_S6) * SSD_S6_LEN + 
                    (i % SSD_CHANNELS_IN_S6) * SSD_S6_CHANNEL_LEN;
        /* the command is next to addr register, each register len is 8 */
        sdev->conf_channel_cmd[i] = sdev->conf_channel_addr[i] + 8;
    }

    /* init timer and synchronization structure of ssd operation */
    for (i = 0; i < SSD_MAX_CHANNEL; i++) {
        pch_ctl = &(sdev->channel_ctl[i]);
        pch_ctl->channel = (ssd_u8)i;         
        init_MUTEX(&(pch_ctl->sema));
        init_timer(&(pch_ctl->timeout)); 
        pch_ctl->redzone0 = 0xffffffff;
        pch_ctl->redzone1 = 0xffffffff;
        pch_ctl->redzone2 = 0xffffffff;
        pch_ctl->redzone3 = 0xffffffff;
        pch_ctl->redzone4 = 0xffffffff;
    }
    return SSD_OK;

}

static void ssd_unset_conf(struct ssd_device *sdev)
{
    int i;

    for (i = 0; i < SSD_MAX_CHANNEL; i++) {
        if (sdev->kbuftable_cpu[i]) {
            pci_unmap_single(sdev->pdev, sdev->kbuftable_dma[i], 
                SSD_BUFFER_SIZE, DMA_BIDIRECTIONAL);
            sdev->kbuftable_cpu[i] = NULL;
        }
    }
    
    //modified at 2012.11.28
    sdev->kbuf_flush = NULL;
}

static int ssd_enable_msi(struct ssd_device *sdev)
{
    int ret;
    struct pci_dev *pdev = sdev->pdev;

    ret = pci_enable_msi(pdev);
    if (ret) {
        printk(KERN_ERR "SSD: enable msi failed\n");
        goto err_out;
    }

    ret = request_irq(pdev->irq, &ssd_interrupt, IRQF_SHARED, sdev->name, sdev);
    if (ret) {
        printk(KERN_ERR "%s: irq register failed\n", sdev->name);
        goto err_irq;
    }

    return SSD_OK;

err_irq:
    pci_disable_msi(pdev);
err_out:
    return SSD_ERROR;
}

static void ssd_disable_msi(struct ssd_device *sdev)
{
    struct pci_dev *pdev = sdev->pdev;

    free_irq(pdev->irq, sdev);
    pci_disable_msi(pdev);
}

static int  __devinit ssd_probe(struct pci_dev *pdev, const struct pci_device_id *ent)
{
    int ret = SSD_OK;

    struct ssd_device *sdev;
    void __iomem *ioaddr;    

    /* start pci device */
    if (pci_enable_device(pdev) < 0) {
        printk(KERN_ERR "%s: can not enable pci device\n", SSD_DEVICE_NAME); 
        ret = -EIO;
        goto err_out;
    }

    pci_set_master(pdev);
    
    /* set 64 DMA model */
    if (pci_set_dma_mask(pdev, DMA_BIT_MASK(64))) {
        printk(KERN_ERR "%s: 64 bits DMA unavailable.\n", SSD_DEVICE_NAME);
        ret = -ENXIO;
        goto err_set_dma64;
    }

    /* map bar0 ssd memory space */
    if (!request_mem_region(pci_resource_start(pdev, SSD_BAR),
        pci_resource_len(pdev, SSD_BAR), SSD_DEVICE_NAME)) {
        printk(KERN_ERR "%s: cannot reserve MEM region 0\n", SSD_DEVICE_NAME);
        ret = -ENODEV;
        goto err_request_mem;
    }
    
    ioaddr = pci_iomap(pdev, SSD_BAR, 0);
    if (!ioaddr) {
        printk(KERN_ERR "%s: cannot remap MEM region 0\n", SSD_DEVICE_NAME);
        ret = -EBUSY;
        goto err_iomap;
    }

    sdev = vmalloc(sizeof(struct ssd_device));
    if (!sdev) {
        printk(KERN_ERR "SSD: memory alloc fail of ssd_device\n");
        ret = -ENOMEM;
        goto err_alloc_device;
    }

    /* init ssd_device struct */
    memset(sdev, 0, sizeof(struct ssd_device));
    strncpy(sdev->name, SSD_DEVICE_NAME, MAX_DEVNAME);
    sdev->conf_regs = ioaddr;    
    sdev->pdev = pdev;
    pci_set_drvdata(pdev, sdev);
    sdev->owner= THIS_MODULE;

    ret = ssd_enable_msi(sdev);
    if (ret) {
        ret = -ENXIO;
        goto err_disable_msi;
    }

    ret = ssd_set_conf(sdev);
    if (ret) {
        ret = -ENOMEM;
        goto err_sys_conf;
    }
    
    ssd_dev = sdev;

    return SSD_OK;

err_sys_conf:
    ssd_disable_msi(sdev);
err_disable_msi:    
    kfree(sdev);
    sdev = NULL;
err_alloc_device:
    pci_iounmap(pdev, ioaddr);
err_iomap:
    release_mem_region(pci_resource_start(pdev, SSD_BAR), pci_resource_len(pdev, SSD_BAR));
err_request_mem:
err_set_dma64:
    pci_disable_device(pdev);
err_out:
    return ret;

}

static void __devexit ssd_remove (struct pci_dev *pdev)
{
    struct ssd_device *sdev = pci_get_drvdata (pdev);

    if (!sdev) {
        printk(KERN_ERR "SSD: sdev is null\n");
        return;
    }

    /* release resouce */
    ssd_unset_conf(sdev);
    
    ssd_disable_msi(sdev);
    pci_iounmap(pdev, sdev->conf_regs);
    release_mem_region(pci_resource_start(pdev, SSD_BAR), pci_resource_len(pdev, SSD_BAR));
    pci_set_drvdata(pdev, NULL) ;
    kfree(sdev);
    pci_disable_device(pdev);
}

static int ssd_create_slab(struct ssd_device *sdev)
{
    /* create slab for hash_node on each channel */
    sdev->hash_node_slab = kmem_cache_create(sdev->name,
            sizeof(struct ssd_hash_node), 0, SLAB_HWCACHE_ALIGN, NULL);
    if (!sdev->hash_node_slab) {
        ssd_log(SSD_LOG_ERR, "create slab failed (line %d)\n", __LINE__);
        return -SSD_ERR_NOMEM;
    }  
    return SSD_OK;
}

static void ssd_init_hash(struct ssd_device *sdev)
{
    int i, j;
    struct ssd_hash_head *phead;
    struct channel_hash_list *p_hash = sdev->ch_hash;

    for (i = 0; i < SSD_MAX_CHANNEL; i++, p_hash++) {
        phead = p_hash->head;
        for (j = 0; j < SSD_HASH_SIZE; j++, phead++) {
            phead->count = ATOMIC_INIT(0);
            spin_lock_init(&(phead->hash_lock));
            phead->first = NULL;
        }
    }
    ssd_log(SSD_LOG_INFO, "init hash table\n");
    
}

static void ssd_destory_slab(struct ssd_device *sdev)
{
    int i, j;
    struct ssd_hash_head *phead;
    struct ssd_hash_node *cur, *next;
    struct channel_hash_list *p_hash = sdev->ch_hash;

    /* free all slab object */
    for (i = 0; i < SSD_MAX_CHANNEL; i++, p_hash++) {
        phead = p_hash->head;
        for (j = 0; j < SSD_HASH_SIZE; j++, phead++) {
            spin_lock(&(phead->hash_lock));
            cur = phead->first;
            while (cur != NULL) {
                next = cur->next;
                kmem_cache_free(sdev->hash_node_slab, cur);
                cur = next;
            }
            phead->first = NULL;
            spin_unlock(&(phead->hash_lock));
        }
    }

    kmem_cache_destroy(sdev->hash_node_slab);
}

static ssd_u16 ssd_init_channel_bitmap(struct free_area_list *area, ssd_u16 start, ssd_u16 pre_alloc)
{
    ssd_u16 free_area_start = start;
    int i, j;

    for (i = 0; i < ALLBLOCKS; i++) {
        area->fab[i].type = i;
        spin_lock_init(&area->fab[i].bitmap_lock);

     /*bitmap: filled with 1
        *0 --- available
        *1 --- unavailable
        */
        bitmap_fill(area->fab[i].area_map, SSD_MAX_VIRBLOCKS_PER_CHANNEL);
     /*clear pre_alloc bit for IBLOCK and DBLOCK
        *IBLOCK:
        *----------------------------------------
        *|00000000000...  |11111111111111111111111|
        *----------------------------------------
        *start           pre_alloc             other
        *DBLOCK:
        *----------------------------------------
        *|11111111111111|01111|01111|...  |11111111|
        *----------------------------------------
        *                        start            pre_alloc  other
        */
        spin_lock(&area->fab[i].bitmap_lock);
        if (DBLOCK == area->fab[i].type) {
            for(j = free_area_start; j < (free_area_start + pre_alloc * BLOCKSIZE[DBLOCK]); j += BLOCKSIZE[DBLOCK])
                bitmap_clear(area->fab[i].area_map, j, 1);
        } else {
            bitmap_clear(area->fab[i].area_map, free_area_start, pre_alloc);
        }
        spin_unlock(&area->fab[i].bitmap_lock);
        if (pre_alloc)
            free_area_start += pre_alloc * BLOCKSIZE[i];
    }
    return free_area_start;
}

/*reload bitmap from hash table*/
static int ssd_reload_bitmap(struct ssd_device *sdev)
{
    struct channel_hash_list *ch_hash;
    struct ssd_hash_head *head;
    struct ssd_hash_node *node;
    struct free_area_list *area;
    ssd_u32 bucket, ret, blocks;
    ssd_u8 channel;
    int start, end, index, i;
    struct semaphore *sema;
    DECLARE_BITMAP(area_bitmap, SSD_MAX_VIRBLOCKS_PER_CHANNEL);
    
    /*scan hash table, and init bitmap*/
    for (channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
        ch_hash = &sdev->ch_hash[channel];
        /*construct area_bitmap*/
        bitmap_zero(area_bitmap, SSD_MAX_VIRBLOCKS_PER_CHANNEL);
        
        for (bucket = 0; bucket < SSD_HASH_SIZE; bucket++) {
            head = &ch_hash->head[bucket];
            spin_lock(&head->hash_lock);
            node = head->first;
            while (node) {
                if (channel != node->channel) {
                    ssd_log(SSD_LOG_ERR, "unmatched channel: %u vs %u\n (line %d)", channel, node->channel, __LINE__);
                    spin_unlock(&head->hash_lock);
                    return SSD_ERROR;
                }
        
                /*set area_bitmap*/
                bitmap_set(area_bitmap, node->block, BLOCKSIZE[node->len]);
                node = node->next;    
            }
            spin_unlock(&head->hash_lock);
        }
    
        area = &sdev->area[channel];
        sema = &sdev->channel_ctl[channel].sema;
        /*empty?*/
        if (bitmap_empty(area_bitmap, SSD_MAX_VIRBLOCKS_PER_CHANNEL)){
            ssd_log(SSD_LOG_INFO, "empty bitmap\n");
            ret = ssd_init_channel_bitmap(area, 0, PRE_ALLOC_BLOCKS);
            down(sema);
            sdev->current_free_area[channel] = ret;
            sdev->free_blocks[channel] = SSD_MAX_VIRBLOCKS_PER_CHANNEL;
            up(sema);
            continue;
        }
        
        /*find current: scan area_bitmap from the end to beginning*/
        for (end = SSD_MAX_VIRBLOCKS_PER_CHANNEL - 1; end >= 0; end--) {
            if (test_bit(end, area_bitmap))
                break;
        }
        ret = end + 1;
        if (end < SSD_MAX_VIRBLOCKS_PER_CHANNEL / 2)
            ret = ssd_init_channel_bitmap(area, ret, PRE_ALLOC_BLOCKS);
        else
            ret = ssd_init_channel_bitmap(area, ret, 0);
    
        /*set current and free blocks*/
        down(sema);
        sdev->current_free_area[channel] = ret;
        ssd_log(SSD_LOG_INFO, "set current_free_area in channel %u: %u\n", channel, ret);
        sdev->free_blocks[channel] = SSD_MAX_VIRBLOCKS_PER_CHANNEL - (end + 1);

        /*init fab: scan area_bitmap from beginning to current*/
        for (i = DBLOCK; i >=IBLOCK; i--) {
            blocks = BLOCKSIZE[i];
            spin_lock(&area->fab[i].bitmap_lock);
            for (start = 0; start <= end;) {
                index = bitmap_find_next_zero_area(area_bitmap, end + 1, start, blocks, 0);
                if (index > end) {
                    ssd_log(SSD_LOG_INFO, "the end of area_bitmap\n");
                    break;
                }
    
                bitmap_set(area_bitmap, index, blocks);
                bitmap_clear(area->fab[i].area_map, index, 1);
    
                start = index + blocks;
                sdev->free_blocks[channel] += blocks;
            }
            spin_unlock(&area->fab[i].bitmap_lock);
        }
        up(sema);
    }

    return SSD_OK;
}

/*scan DBLOCK list and split it*/
static int bitmap_split(struct free_area_bitmap *from, struct free_area_bitmap *to)
{
    int ret = -1;
    ssd_u16 blocks = BLOCKSIZE[from->type];    
    
    if ((DBLOCK != from->type) || (IBLOCK != to->type))
        return SSD_ERROR;
    
    spin_lock(&to->bitmap_lock);
    spin_lock(&from->bitmap_lock);
    
    ret = bitmap_find_next_zero_area(from->area_map, SSD_MAX_VIRBLOCKS_PER_CHANNEL, PRE_ALLOC_START, 1, 0);
    if (ret >= SSD_MAX_VIRBLOCKS_PER_CHANNEL) {
        ssd_log(SSD_LOG_INFO, "there is no free blocks: %d\n", ret);
        ret = SSD_ERROR;
        goto error;
    }
    
  /*set DBLOCK and clear IBLOCK
    *DBLOCK:1111111|01111|1111|01111|11...
    *                 \/
    *IBLOCK:1101011|00000|0111|11111|11...
    *DBLOCK:1111111|11111|1111|01111|11...
    */
    bitmap_set(from->area_map, ret, 1);
    bitmap_clear(to->area_map, ret, blocks);
error:
    spin_unlock(&from->bitmap_lock);
    spin_unlock(&to->bitmap_lock);

    return ret;
}


/*scan IBLOCK list and merge to DBLOCK*/
static int bitmap_merge(struct free_area_bitmap *from, struct free_area_bitmap *to)
{
    int index, start;
    ssd_u16 count = 0, blocks = BLOCKSIZE[to->type];
    
    if ((IBLOCK != from->type) || (DBLOCK != to->type)) 
        return SSD_ERROR;

    ssd_log(SSD_LOG_INFO, "bitmap_merge: 2M block to 10M block\n");
    
    spin_lock(&from->bitmap_lock);
    spin_lock(&to->bitmap_lock);
    for (start = PRE_ALLOC_START; start < SSD_MAX_VIRBLOCKS_PER_CHANNEL;) {
        index = bitmap_find_next_zero_area(from->area_map, SSD_MAX_VIRBLOCKS_PER_CHANNEL, start, blocks, 0);
        if (index >= SSD_MAX_VIRBLOCKS_PER_CHANNEL)
            break;
                
     /*set IBLOCK and clear DBLOCK
        *IBLOCK:1101011|00000|0111|00000|11...
        *                 \/        \/
        *DBLOCK:1111111|01111|1111|01111|11...
        *IBLOCK:1101011|11111|0111|11111|11...
        */
        bitmap_set(from->area_map, index, blocks);
        bitmap_clear(to->area_map, index, 1);
        count++;
        start = index + blocks;
    }
    
    spin_unlock(&to->bitmap_lock);
    spin_unlock(&from->bitmap_lock);
    ssd_log(SSD_LOG_INFO, "merge to %u 10M blocks\n", count);
    
    return count;
}

static int ssd_remap_block(struct ssd_device *sdev, ssd_u8 channel)
{
    int i, j, ret = -1, curr_pos = 0;
    ssd_u32 index;
    ssd_u8 *map, *start;
    struct ssd_table_param stparam;
    struct free_area_list *area = &sdev->area[channel];    
    struct ssd_hash_head *head;
    struct ssd_hash_node *node;         

    ssd_log(SSD_LOG_INFO, "ssd_remap_block in channel: %u\n", channel);
    
    stparam.usr_buf = NULL;
    stparam.channel = channel;
    stparam.type = TABLE_ADDR_TYPE;
    
    /*step 1: remap ssd*/    
    /*get addr table from ram*/
    ret = ram_table_read(sdev, &stparam);
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "fail to read ram table, channel = %u type = %u ,ret[%d] (line %d)\n", stparam.channel, stparam.type, ret, __LINE__);
        goto out;
    }

    start = sdev->kbuftable_cpu[channel];
    map = vmalloc(SSD_TABLE_SIZE);
    if (!map) {
        ssd_log(SSD_LOG_ERR, "vmalloc map failed (line %d)\n", __LINE__);
        ret = -SSD_ERR_NOMEM;
        goto out;
    }

    memset(map, 0, SSD_TABLE_SIZE);
    memcpy(map, start, SSD_TABLE_SIZE); 
    memset(start, 0, SSD_TABLE_SIZE);
    
    for (i = 0; i < SSD_HASH_SIZE; i++) {
        head = sdev->ch_hash[channel].head + i;
        spin_lock(&head->hash_lock);
        node = head->first;
        
        while (node) {
            index = node->block;
            node->block = curr_pos;
            for (j = 0; j < BLOCKSIZE[node->len]; j++) {
                memcpy(start + 2 * curr_pos, map + 2 * (index + j), 2);
                curr_pos++;
            }
            node = node->next;
        }
    }   
    
    sdev->current_free_area[channel] = curr_pos;
    ssd_log(SSD_LOG_INFO, "after remap in channel %u, current: %d\n", channel, curr_pos);
    /*flush mapper table to fpga*/
    ssd_write_tables_to_ram(sdev, channel, SSD_FLAG_WRITE_ADDR_TABLE);
    
    /*step 2: update hash table to ram*/    
    for (i = 0; i < SSD_HASH_SIZE; i++) {
        head = sdev->ch_hash[channel].head + i;
        spin_unlock(&head->hash_lock);        
    }
    
    /*step 3: init bitmap*/
    ret = ssd_init_channel_bitmap(area, curr_pos, 0);
    kfree(map);

out:   
    return ret;
}

static int ssd_alloc_logic_block(struct ssd_device *sdev, ssd_u8 channel, ssd_u16 type)
{
    struct free_area_list *area = sdev->area + channel;
    struct free_area_bitmap *fab;
    int ret = 0;
    //int mapped = 0;
    ssd_u16 blocks;
    int index; 
    
    switch (type) {
        case IBLOCK:
            ssd_log(SSD_LOG_INFO, "alloc 2M block\n");
            fab = area->fab + IBLOCK;
            blocks = BLOCKSIZE[IBLOCK];
alloc_iblock_retry:
            spin_lock(&fab->bitmap_lock);
            /*get block from fab*/
            index = bitmap_find_next_zero_area(fab->area_map, SSD_MAX_VIRBLOCKS_PER_CHANNEL, 0, 1, 0);
            if (index < SSD_MAX_VIRBLOCKS_PER_CHANNEL) {
                bitmap_set(fab->area_map, index, 1);
                spin_unlock(&fab->bitmap_lock);
               
                sdev->free_blocks[channel] -= blocks;  
                return index;
            }
    
            /*split DBLOCK */
            spin_unlock(&fab->bitmap_lock);
            ret = bitmap_split(fab + DBLOCK, fab);
            if (ret >= 0)
                goto alloc_iblock_retry;
    
            /*alloc block from SSD at current*/
            if ((sdev->current_free_area[channel] + blocks) <= SSD_MAX_VIRBLOCKS_PER_CHANNEL) { 
                ssd_log(SSD_LOG_INFO, "alloc blocks from current %u\n", 
                        sdev->current_free_area[channel]);               
                index = sdev->current_free_area[channel];
                sdev->current_free_area[channel] += blocks;
                sdev->free_blocks[channel] -= blocks;    
               
                return index;
            }
    
            break;
            
        case DBLOCK:
            ssd_log(SSD_LOG_INFO, "alloc 10M block\n");
            fab = area->fab + DBLOCK;
            blocks = BLOCKSIZE[fab->type];
alloc_dblock_retry:
            spin_lock(&fab->bitmap_lock);
            /*get block from fab*/
            index = bitmap_find_next_zero_area(fab->area_map, SSD_MAX_VIRBLOCKS_PER_CHANNEL, 0, 1, 0);
            if(index < SSD_MAX_VIRBLOCKS_PER_CHANNEL) {
                bitmap_set(fab->area_map, index, 1);
                spin_unlock(&fab->bitmap_lock);                
                sdev->free_blocks[channel] -= blocks;       
                
                return index;
            }
            
            spin_unlock(&fab->bitmap_lock);    
            /*alloc block from SSD by current*/           
        
            /*current <= throttle*/
          //  if((sdev->current_free_area[channel] <= CURRENT_AREA_THROTTLE || 
                /*if all_free_bocks - throttle <= reclaim_throttle, we ignore it; else we reclaim it*/
            /*
                (sdev->free_blocks[channel] - 
                (SSD_MAX_VIRBLOCKS_PER_CHANNEL - sdev->current_free_area[channel]))
                <= FREE_AREA_RECLAIM_THROTTLE) && 
            */
            if ((sdev->current_free_area[channel] + blocks) <= SSD_MAX_VIRBLOCKS_PER_CHANNEL) {
                ssd_log(SSD_LOG_INFO, "alloc blocks from current %u\n", 
                        sdev->current_free_area[channel]);
                index = sdev->current_free_area[channel];
                sdev->current_free_area[channel] += blocks;
                sdev->free_blocks[channel] -= blocks;    
                
                return index;
            }
    
            /*the first reclaim: merge IBLOCK list to DBLOCK list*/            
            ret = bitmap_merge(area->fab + IBLOCK, fab);
            if(ret > 0)
                goto alloc_dblock_retry;
    
            /*unexpected, the worst case*/
            /*
            if(!mapped && sdev->free_blocks[channel] >= blocks) {
                ret = ssd_remap_block(sdev, channel);
                ssd_log(SSD_LOG_INFO, "free blocks %d\n", sdev->free_blocks[channel]);
                mapped = 1;
                if(ret < 0){
                    goto alloc_error;
                }
    
                goto alloc_dblock_retry;
            }
            */
                
            break;
    }

//alloc_error:
    ssd_log(SSD_LOG_ERR, "fail to alloc block (line %d)\n", __LINE__);
    return -SSD_ERR_ALLOC_BLOCK;
}

static int ssd_sys_open(struct ssd_device *sdev)
{
    int ret;
    int channel;
    
    if (g_opened_flag == SSD_OPENED)
        return -SSD_ERR_NOT_CLOSED;

    g_ssd_log_fp = ssd_log_open(LOG_PATH, O_CREAT | O_RDWR | O_APPEND, 0);
    if (!g_ssd_log_fp)
        printk(KERN_INFO "ssd_log_open failed\n");
    
    ret = ssd_create_slab(sdev);
    if (SSD_OK != ret) {
        ssd_log(SSD_LOG_ERR, "ssd_create_slab error: %d (line %d)\n", ret, __LINE__);
        goto out;
    }

    ssd_init_hash(sdev);
    /* init hash table head for each channel */ 
    ret = ssd_sys_init(sdev);
    if (SSD_OK != ret){
        ssd_log(SSD_LOG_ERR, "ssd_sys_init error: %d (line %d)\n", ret, __LINE__);
        goto err;
    }
    
    // ssd_load_hash_table(sdev);
    ret = ssd_reload_bitmap(sdev);
    if (SSD_OK != ret) {
        ssd_log(SSD_LOG_ERR, "ssd_reload_bitmap error: %d (line %d)\n", ret, __LINE__);
        goto err;
    }

    /*set for free space*/
    for (channel = 0; channel < SSD_MAX_CHANNEL; channel++)
        free_blocks[channel] = sdev->free_blocks[channel];

	/*init flush semaphore*/
	init_MUTEX(&flush_sema);

    g_opened_flag = SSD_OPENED;

    return SSD_OK;

err:
    ssd_destory_slab(sdev);
    
out:
    ssd_log_close();
    return SSD_ERROR;
}

static int ssd_sys_close(struct ssd_device *sdev)
{
    int ret;
    
    if (g_opened_flag != SSD_OPENED)
        return SSD_OK;// -SSD_ERR_NOT_OPENED;
        
    ret = ssd_sys_flush(sdev, FLUSH_CLOSE_TYPE);

    g_opened_flag = SSD_CLOSED;

    ssd_destory_slab(sdev);
    ssd_log_close();

    return ret;
}

static void ssd_timeout(channel_op_control_t *ch_ctl)
{
    ch_ctl->errors = -SSD_ERR_TIMEOUT;

    /* interrupt may receive this time */
    if (ch_ctl->waiting) {
        complete(ch_ctl->waiting);
        ch_ctl->waiting = NULL;
    }
}

static void ssd_add_timer(channel_op_control_t *ch_ctl, unsigned long timeout, 
                               void (*func)(channel_op_control_t *))
{
    struct timer_list *timer = &(ch_ctl->timeout);;

    if (timer->function) {
        del_timer(timer);
    }
    timer->data = (unsigned long)ch_ctl;
    timer->expires = jiffies + timeout;
    timer->function = (void (*)(unsigned long))func;
    add_timer(timer);
}

static int channel_op(struct ssd_device *sdev, ssd_u8 channel, ssd_u64 cmd)
{
    long ret;
    ssd_u8 tmp;
    int loop;
    DECLARE_COMPLETION(wait);
    channel_op_control_t *ch_ctl = &(sdev->channel_ctl[channel]);

    BUG_ON(ch_ctl->redzone0 != 0Xffffffff);
    BUG_ON(ch_ctl->redzone1 != 0Xffffffff);
    BUG_ON(ch_ctl->redzone2 != 0Xffffffff);
    BUG_ON(ch_ctl->redzone3 != 0Xffffffff);
    BUG_ON(ch_ctl->redzone4 != 0Xffffffff);
    /* init the operation status */
    ch_ctl->errors = -SSD_ERR_WAIT;

    tmp = (ssd_u8)(cmd & 0x0f);
    if (3 != tmp && 7 != tmp && 8 != tmp && 9 != tmp) {
        ch_ctl->error_ram = 0;
        ssd_add_timer(ch_ctl, SSD_CHANNEL_TIMEOUT, ssd_timeout);
    } else
        ch_ctl->error_ram = -100;

    /* write the dma address to addr register to notify the hardware */
    ssd_reg64_write(sdev->conf_channel_addr[channel], sdev->kbuftable_dma[channel]);

    /* write the operation param to control register to notify the hardware */
    ssd_reg64_write(sdev->conf_channel_cmd[channel], cmd);
    
    if (3 == tmp || 7 == tmp || 8 == tmp || 9 == tmp) {
        for (loop = 0; loop < 400000000; loop++){
            if (-100 != ch_ctl->error_ram) {
                break;    
            }
        }

        if (400000000 == loop) {
            return -49;
        }
        
        return ch_ctl->error_ram;
    }
    
    ch_ctl->waiting = &wait;
    ret = wait_for_completion_timeout(ch_ctl->waiting, 8000);

    /* Wake up by interrupt or timer*/
    ch_ctl->waiting = NULL;

    return ch_ctl->errors;

}

/* just decide which channel to put the hash node */
static inline int id2channel(struct ssd_blockid *id)
{
    /*get channel from id High bits*/
    //return id->m_nLow % SSD_MAX_CHANNEL;
    return (id->m_nLow % SSD_S6_NUMS) * SSD_CHANNELS_IN_S6 + (id->m_nLow / SSD_S6_NUMS) % SSD_CHANNELS_IN_S6;
}
static inline int id_hash(struct ssd_blockid *id)
{
    /*get bucket from id Low bits*/
    return id->m_nLow % SSD_HASH_SIZE;
}

/*insert element to  hash table*/
static int set_id_info(struct ssd_device *sdev, struct ssd_blockid *id, id_info_t *info)
{
    int hash_list;
    struct ssd_hash_node *node;
    struct ssd_hash_head *head;
    struct channel_hash_list *ch_hash;
    
    ch_hash = &sdev->ch_hash[info->channel];
    hash_list = id_hash(id);
    head = &ch_hash->head[hash_list];

    ssd_log(SSD_LOG_INFO, "set_id_info, channel: %u\tblock: %u\tsize:%d\n", 
            info->channel, info->block, BLOCKSIZE[info->len]);
    
    /*alloc node*/
    node = kmem_cache_alloc(sdev->hash_node_slab, GFP_KERNEL);
    memset(node, 0, sizeof(struct ssd_hash_node));
    if (!node)
        return -SSD_ERR_NOMEM;
    
    node->id = *id;
    node->channel = info->channel;
    node->block = info->block;
    node->len = info->len;
    node->next = NULL;
    
    spin_lock(&head->hash_lock);
    node->next = head->first;
    head->first = node;
    atomic_inc(&head->count);
    spin_unlock(&head->hash_lock);
    
    return SSD_OK;
}

/*construct info from hash nodes*/
static int get_id_info(struct ssd_device *sdev, struct ssd_blockid *id, id_info_t *info)
{
    ssd_u8 hash_channel;
    int hash_list;
    struct ssd_hash_node *node;
    struct ssd_hash_head *head;
    struct channel_hash_list *ch_hash;
 
    hash_channel = (ssd_u8)id2channel(id);
    ch_hash = &(sdev->ch_hash[hash_channel]);
    hash_list = id_hash(id);
    head = &(ch_hash->head[hash_list]);

    spin_lock(&(head->hash_lock));
    node = head->first;
    while (node) {    
        prefetch(node->next);
        if ((node->id.m_nHigh == id->m_nHigh) && (node->id.m_nLow == id->m_nLow)) {
            info->len = node->len;
            info->block = node->block;
            info->channel = node->channel;
            break;
        }
        node = node->next;
    }
    spin_unlock(&(head->hash_lock));

    if (!node)
        return -SSD_ID_NO_EXIST;

    ssd_log(SSD_LOG_INFO, "get_id_info, channel: %u\tblock: %u\tsize:%d\n", 
            info->channel, info->block, BLOCKSIZE[info->len]);

    return SSD_OK;
}

static int del_id_info(struct ssd_device *sdev, struct ssd_blockid *id, id_info_t *info)
{
    ssd_u8 hash_channel;
    int hash_list;
    struct ssd_hash_node *pre_node, *cur_node;
    struct ssd_hash_head *head;
    struct channel_hash_list *ch_hash;
 
    hash_channel = (ssd_u8)id2channel(id);
    ch_hash = &(sdev->ch_hash[hash_channel]);
    hash_list = id_hash(id);
    head = &(ch_hash->head[hash_list]);

    spin_lock(&(head->hash_lock));
    pre_node = cur_node = head->first;
    while (cur_node) {
        prefetch(cur_node->next);
        if ((cur_node->id.m_nHigh == id->m_nHigh) && (cur_node->id.m_nLow == id->m_nLow)) {
            info->len = cur_node->len;
            info->block = cur_node->block;
            info->channel = cur_node->channel;
            break;
        }
        pre_node = cur_node;
        cur_node = cur_node->next;
    }
   
    if (!cur_node) {
        spin_unlock(&(head->hash_lock));
        return -SSD_ID_NO_EXIST;
    } else if (pre_node == cur_node) {
        /* the first node in list is the id will be deleted */
        head->first = cur_node->next;
    } else {
        pre_node->next = cur_node->next;
    }
    atomic_dec(&head->count);
    spin_unlock(&(head->hash_lock));

    /* free the hash node */
    kmem_cache_free(sdev->hash_node_slab, cur_node);
       
    ssd_log(SSD_LOG_INFO, "del_id_info, channel: %u\tblock: %u\tsize:%d\n", 
            info->channel, info->block, BLOCKSIZE[info->len]);
    return  SSD_OK;
}

static int check_id_info(id_info_t *id_info)
{
    if (unlikely(id_info->channel >= SSD_MAX_CHANNEL)) {
        ssd_log(SSD_LOG_ERR, "channel[%u] is larger than SSD_MAX_CHANNEL (line %d)\n", id_info->channel, __LINE__);
        return -SSD_ID_MAP_ERROR;
    }

    if (unlikely(id_info->block >= SSD_MAX_VIRBLOCKS_PER_CHANNEL)) {
        ssd_log(SSD_LOG_ERR, "block[%u] is larger than SSD_MAX_VIRBLOCKS_PER_CHANNEL (line %d)\n", id_info->block, __LINE__);
        return -SSD_ID_MAP_ERROR;
    }

    if (unlikely(id_info->len >= ALLBLOCKS)) {
        ssd_log(SSD_LOG_ERR, "invalid value of len[%u]\n", id_info->len);
        return -SSD_ID_MAP_ERROR;
    }
    if (unlikely(id_info->block + BLOCKSIZE[id_info->len] > SSD_MAX_VIRBLOCKS_PER_CHANNEL)) {
        ssd_log(SSD_LOG_ERR, "block + offset overflow, block[%u], blocksize[%u] (line %d)\n", id_info->block, BLOCKSIZE[id_info->len], __LINE__);
        return -SSD_ID_MAP_ERROR;
    }

    return SSD_OK;
}
/*
static int ssd_id_write_retry(struct ssd_device *sdev, struct ssd_write_retry_param * wrparam)
{
    int ret;
    ssd_u64 cmd;
    ssd_u8 channel;
    id_info_t id_info;
    struct semaphore *sema;

    channel = (ssd_u8)id2channel(&wrparam->id);
    sema = &sdev->channel_ctl[channel].sema;
    down(sema);
    
    memset(&id_info, 0, sizeof(id_info_t));
    ret = get_id_info(sdev, &(wrparam->id), &id_info);
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "ssd_id_write_retry: get_id_info failed, ret[%d], id.High=%llx id.Low=%llx channel=%u block=%u len=%u (line %d)\n",
                            ret, wrparam->id.m_nHigh, wrparam->id.m_nLow, id_info.channel, id_info.block, id_info.len, __LINE__);
        ret = -SSD_ID_NO_EXIST;
        goto out;
    }    
    
    cmd = cmd_value((id_info.block + wrparam->block_offset) * SSD_PAGES_PER_BLOCK, wrparam->len * SSD_PAGES_PER_BLOCK, SSD_FLAG_WRITE);
    
    if (copy_from_user(sdev->kbuftable_cpu[id_info.channel], wrparam->buf, wrparam->len * SSD_BLOCK_SIZE)) {
        ssd_log(SSD_LOG_ERR, "ssd_id_write_retry: copy from user failed (line %d)\n", __LINE__);
        ret = -SSD_ERR_COPY_TO_USER;
        goto out;
    }
    
    ret  = channel_op(sdev, id_info.channel, cmd);
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "ssd_id_write_retry: channel_op failed, ret[%d], id.High=%llx id.Low=%llx channel=%u block=%u len=%u (line %d)\n",
                            ret, wrparam->id.m_nHigh, wrparam->id.m_nLow, id_info.channel, id_info.block, id_info.len, __LINE__);
        goto out;
    }

out:
    up(sema);
    return ret;
}
*/
static int ssd_id_write(struct ssd_device *sdev, struct ssd_write_param *wparam)
{
    int ret, index, i;
    //int block_ret_flag, offset;
    //ssd_u8 fail_flag = 1;
    //ssd_u8 retry_times;
    ssd_u8 channel;
    ssd_u64 write_cmd, erase_cmd;
    ssd_u16 type;
    id_info_t id_info;
    struct semaphore *sema;
    int cpu;
    unsigned long tot_ticks1 = 0, tot_ticks2 = 0;
   
    channel = (ssd_u8)id2channel(&wparam->id);
    /*check type*/
    if (BLOCKSIZE[IBLOCK] == wparam->len)
        type = IBLOCK;
    else if (BLOCKSIZE[DBLOCK] == wparam->len)
        type = DBLOCK;
    else {
        ssd_log(SSD_LOG_ERR, "ssd_id_write: SSD_ERROR_PARAM_WRLEN,"
                             "len = %u (line %d)\n", wparam->len, __LINE__);
        return -SSD_ERR_PARAM_WRLEN;
    }    

    sema = &sdev->channel_ctl[channel].sema;
    down(sema);
    
    ret = get_id_info(sdev, &(wparam->id), &id_info);
    if (ret != -SSD_ID_NO_EXIST) {
        ssd_log(SSD_LOG_ERR, "ssd_id_write: get_id_info failed, ret[%d],id.High=%llx"
                             " id.Low=%llx channel=%u block=%u len=%u (line %d)\n",
                             ret, wparam->id.m_nHigh, wparam->id.m_nLow,
                             channel, id_info.block, id_info.len, __LINE__);
        ret = -SSD_ID_ALREADY_EXIST;
        goto out;
    }
    
    /*alloc logic block*/
    index = ssd_alloc_logic_block(sdev, channel, type);
    if (index == -SSD_ERR_ALLOC_BLOCK || index >= SSD_MAX_VIRBLOCKS_PER_CHANNEL) {
        ssd_log(SSD_LOG_ERR, "ssd_id_write: alloc logic block failed,"
                             "index[%d] (line %d)\n", index, __LINE__);
        ret = -SSD_ERR_NO_INVALID_BLOCKS;
        goto out;
    }
    ssd_log(SSD_LOG_INFO, "get free block: %d\n", index);    
    
    memset(&id_info, 0, sizeof(id_info_t));
    id_info.block = index;
    id_info.channel = channel;
    id_info.len = type; 

       
    /* set for cmd "ssdstat" */
    write_count[channel] += ((wparam->len) << (SSD_BLOCK_SHIFT - SSD_PAGE_SHIFT));
    /* set for free space */
    free_blocks[channel] = sdev->free_blocks[channel];
    
    if (copy_from_user(sdev->kbuftable_cpu[channel], 
                       wparam->buf, wparam->len * SSD_BLOCK_SIZE)) {
        ssd_log(SSD_LOG_ERR, "ssd_id_write: copy from user failed (line %d)\n", __LINE__);
        ret = -SSD_ERR_COPY_TO_USER;
        goto fail;
    }
    
    write_cmd = cmd_value(index * SSD_PAGES_PER_BLOCK, 
                          wparam->len * SSD_PAGES_PER_BLOCK, SSD_FLAG_WRITE);
    erase_cmd = cmd_value(index, wparam->len, SSD_FLAG_ERASE);
    

    tot_ticks1 = jiffies;
    for (i = 0; i < 5; i++) {
        ret = channel_op(sdev, channel, write_cmd);
        if (ret == SSD_OK)
            break;
        ssd_log(SSD_LOG_WARNING, "ssd_id_write: ret[%d], times[%d] (line %d)\n", ret, i, __LINE__);
        if (channel_op(sdev, channel, erase_cmd) != SSD_OK)
            break;
    } 
    tot_ticks2 = jiffies;

    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_WARNING, "ssd_id_write: channel op failed, ret[%d], id.High=%llx " 
                                 "id.Low=%llx channel=%u block=%u len=%u (line %d)\n",
                                  ret, wparam->id.m_nHigh, wparam->id.m_nLow, id_info.channel, 
                                  id_info.block, id_info.len, __LINE__);
        goto fail;       
    }

    /* construct hash node and insert to hash table */
    ret = set_id_info(sdev, &wparam->id, &id_info);
    if (ret < 0) {
        ssd_log(SSD_LOG_ERR, "ssd_id_write: set_id_info failed, ret[%d], id.High=%llx " 
                             "id.Low=%llx channel=%u block=%u len=%u (line %d)\n",
                              ret, wparam->id.m_nHigh, wparam->id.m_nLow, id_info.channel,
                              id_info.block, id_info.len, __LINE__);
        ret = -SSD_ERR_NOMEM;
        goto no_mem;
    }
    
    up(sema);
    
    /* set for cmd "iostat" */
    cpu = part_stat_lock();
    part_stat_add(cpu, &(sdev->gdisk[0]->part0), ios[1], 1);
    part_stat_add(cpu, &(sdev->gdisk[0]->part0), merges[1], 1);
    part_stat_add(cpu, &(sdev->gdisk[0]->part0), sectors[1], 
                  (wparam->len) * 2 * 1024 * 2);      
    part_stat_add(cpu, &(sdev->gdisk[0]->part0), io_ticks,
                         (tot_ticks2 - tot_ticks1) /  SSD_MAX_CHANNEL);
    part_stat_unlock();

    return ret;

no_mem:
    channel_op(sdev, channel, erase_cmd);
fail:
    delete_logic_block(sdev, &id_info);
out:
    up(sema);
    return ret;
}

static int ssd_id_read(struct ssd_device *sdev, struct ssd_read_param *rparam)
{
    int ret;
    ssd_u8 channel;
    ssd_u64 cmd;
    id_info_t id_info;
    struct semaphore *sema;
    int cpu;
    unsigned long tot_ticks1 = 0, tot_ticks2 = 0;
    
    memset(&id_info, 0, sizeof(id_info_t));
    channel = (ssd_u8)id2channel(&rparam->id);
    sema = &(sdev->channel_ctl[channel].sema);
    down(sema);
    
    ret = get_id_info(sdev, &(rparam->id), &id_info);
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "ssd_id_read: get_id_info failed, ret[%d], id.High=%llx id.Low=%llx channel=%u block=%u len=%u (line %d)\n",
                            ret, rparam->id.m_nHigh, rparam->id.m_nLow, channel, id_info.block, id_info.len, __LINE__);
        ret = -SSD_ID_NO_EXIST;
        goto out;
    }

    ret = check_id_info(&id_info);
    if (unlikely(ret != SSD_OK)) {
        ssd_log(SSD_LOG_WARNING, "id(%llu:%llu) information in id table is error\n",
            rparam->id.m_nHigh, rparam->id.m_nLow);
        goto out;
    }
    
    /*align by page*/
    cmd = cmd_value(id_info.block * SSD_PAGES_PER_BLOCK + rparam->offset,
                    rparam->len, SSD_FLAG_READ);
    
    read_count[channel] += (rparam->len);

    tot_ticks1 = jiffies;    
    ret = channel_op(sdev, channel, cmd);
    tot_ticks2 = jiffies;
    
    if ((ret & 0x7f) != SSD_OK) { 
        ssd_log(SSD_LOG_ERR, "ssd_id_read: channel op failed, ret[%d], id.High=%llx id.Low=%llx channel=%u block=%u len=%u (line %d)\n",
                            ret, rparam->id.m_nHigh, rparam->id.m_nLow, id_info.channel, id_info.block, id_info.len, __LINE__);
        goto out;
    }
    
    if (copy_to_user(rparam->buf, sdev->kbuftable_cpu[channel], rparam->len * SSD_PAGE_SIZE)) {
        ssd_log(SSD_LOG_ERR, "ssd_id_read: copy to user failed (line %d)\n", __LINE__);
        ret = -SSD_ERR_COPY_TO_USER;
    }
  
out:
    up(sema);
    
    /* set for cmd "iostat" */
    cpu = part_stat_lock();
    part_stat_add(cpu, &(sdev->gdisk[0]->part0), ios[0], 1);
    part_stat_add(cpu, &(sdev->gdisk[0]->part0), merges[0], 1);
    part_stat_add(cpu, &(sdev->gdisk[0]->part0), sectors[0], (rparam->len) * 16);
    part_stat_add(cpu, &(sdev->gdisk[0]->part0), io_ticks, (tot_ticks2 - tot_ticks1) / SSD_MAX_CHANNEL);
    part_stat_unlock();

    return ret;
}

/*free space to SSD(IBLOCK list, DBLOCK list, SSD)*/
static void delete_logic_block(struct ssd_device *sdev, id_info_t *info)
{
    struct free_area_bitmap *fab = sdev->area[info->channel].fab + info->len;    
    
    ssd_log(SSD_LOG_INFO, "delete logic block\n");
    spin_lock(&fab->bitmap_lock);
    bitmap_clear(fab->area_map, info->block, 1);
    /*merge?*/
    spin_unlock(&fab->bitmap_lock);
        
    sdev->free_blocks[info->channel] += BLOCKSIZE[info->len];
}

static int ssd_id_delete(struct ssd_device *sdev, struct ssd_blockid *id)
{
    int ret;
    ssd_u8 channel;
    ssd_u32 len;    
    ssd_u64 cmd;
    id_info_t id_info;
    struct semaphore *sema;
   
    memset(&id_info, 0, sizeof(id_info_t));
    channel = (ssd_u8)id2channel(id);
    sema = &(sdev->channel_ctl[channel].sema);
    down(sema);
    /*delete from hash table*/
    ret = del_id_info(sdev, id, &id_info);
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "ssd_id_delete: del_id_info failed, ret[%d], id.High=%llx id.Low=%llx channel=%u block=%u len=%u (line %d)\n",
                            ret, id->m_nHigh, id->m_nLow, id_info.channel, id_info.block, id_info.len, __LINE__);
        ret = -SSD_ID_NO_EXIST;
        goto out;
    }

    ret = check_id_info(&id_info);
    if (unlikely(ret != SSD_OK)) {
        ssd_log(SSD_LOG_WARNING, "id(%llu:%llu) information in id table is error,ret[%d] id.High=%llx id.Low=%llx channel=%u block=%u len=%u (line %d)\n",
                            ret, id->m_nHigh, id->m_nLow, id_info.channel, id_info.block, id_info.len, __LINE__);
        if(SSD_OK != set_id_info(sdev, id, &id_info))
            ssd_log(SSD_LOG_ERR, "redo set id info error\n");
        
        goto out;
    }    

    len = (ssd_u32)((id_info.len == 0) ? BT_INDEX_BLOCK_NUM : BT_DATA_BLOCK_NUM);
    cmd = cmd_value(id_info.block, len, SSD_FLAG_ERASE);   
    
    delete_count[channel] += (len << (SSD_BLOCK_SHIFT - SSD_PAGE_SHIFT));
    
    ret = channel_op(sdev, channel, cmd);    
    
    if (unlikely(ret != SSD_OK)) {
        ssd_log(SSD_LOG_ERR, "erase block failed: ret[%d], id.High=%llx id.Low=%llx channel=%u block=%u len=%u (line %d)\n",
                            ret, id->m_nHigh, id->m_nLow, id_info.channel, id_info.block, id_info.len, __LINE__);
        if(SSD_OK != set_id_info(sdev, id, &id_info))
            ssd_log(SSD_LOG_ERR, "redo set id info error\n");
        
        goto out;
    }

    delete_logic_block(sdev, &id_info);

    /*set for free space*/
    free_blocks[channel] = sdev->free_blocks[channel];

out:
    up(sema);
    return ret;

}

/* step 1 of flush process: copy address table and erase table to meomory */ 
static int copy_ram_table(struct ssd_device *sdev, ssd_u8 channel, char *buffer)
{
    int ret;
    struct ssd_table_param param;
    struct semaphore *sema = &(sdev->channel_ctl[channel].sema);
    
    param.channel = channel;
    
    down(sema);
    /*the addr table and erase table in one channel are matched */
    /* copy addrss table */
    param.type = TABLE_ADDR_TYPE;
    ret = ram_table_read(sdev, &param);
    if (unlikely(ret != SSD_OK)) 
        goto out;
    memcpy(buffer, sdev->kbuftable_cpu[channel], SSD_TABLE_SIZE);

    /* copy erase table */
    param.type = TABLE_ERASE_TYPE;
    ret = ram_table_read(sdev, &param);
    if (unlikely(ret != SSD_OK)) 
        goto out; 
    memcpy(buffer + SSD_TABLE_SIZE, sdev->kbuftable_cpu[channel], SSD_TABLE_SIZE);

out:
    up(sema);
    
    return ret;
}

/* inline function in "copy_id_hash": copy hash_node to flush_id_info */
static inline void copy_hash_node_to_flush_id_info(struct ssd_hash_node *node, struct flush_id_info *sp)
{
    ssd_u8 channel;
    sp->id = node->id;
    sp->channel = node->channel;
    channel = (ssd_u8)id2channel(&sp->id);
    if (sp->channel != channel) {
        ssd_log(SSD_LOG_WARNING, "id vs channel illegal: id.High=%llx id.Low=%llx channel(actual)=%u channel(legal)=%u (line %d)\n",
            sp->id.m_nHigh, sp->id.m_nLow, sp->channel, channel, __LINE__);
        sp->channel = channel;
    }
    sp->block = node->block;
    sp->len = node->len;
    sp->crc32 = 0;
}

/* step 2 of flush process: copy id hash table to memory */ 
static ssd_u32 copy_id_hash(struct ssd_device *sdev, struct flush_id_info *buffer)
{
    ssd_u8 channel;
    int hash_list;
    ssd_u32 id_num = 0;
    struct ssd_hash_node *node;
    struct ssd_hash_head *head;
    struct flush_id_info *sp = buffer;
    struct channel_hash_list *ch_hash;

    for (channel = 0; channel < SSD_MAX_CHANNEL; channel++) { 
        ch_hash = &(sdev->ch_hash[channel]);
        for (hash_list = 0; hash_list < SSD_HASH_SIZE; hash_list++) {
            head = &(ch_hash->head[hash_list]);
            
            spin_lock(&(head->hash_lock));
            node = head->first;
            while (node) {
                prefetch(node->next);
                copy_hash_node_to_flush_id_info(node, sp);
                sp++;
                id_num++;
                node = node->next;
            }          
            spin_unlock(&(head->hash_lock));
        }
    }
    
    return id_num;
}

/* search next normal block in reserved block */ 
static inline ssd_s32 find_new_table_pos(struct ssd_device *sdev, ssd_u16 cur_pos)
{
    ssd_u16 bitmap_pos;
    
    /* bitmap for tabledata start from channel 2 */
    bitmap_pos = cur_pos - RESERVE_BLOCKS_META;
    bitmap_pos = (bitmap_pos + 1) % TABLE_BITMAP_SIZE;
    return (bitmap_pos + RESERVE_BLOCKS_META);
}


/* calculate physical block in tabledata */
static inline ssd_s32 calc_phyblock(ssd_u16 block_pos)
{
    ssd_u8 pos_in_channel;
    ssd_u32 phyblock;
    ssd_u16 plane_table_start[SSD_MAX_PLANES] = { PLANE0_TABLE_START,
                                                  PLANE1_TABLE_START, 
                                                  PLANE2_TABLE_START,
                                                  PLANE3_TABLE_START };
    /* check block_pos */
    if (block_pos >= RESERVE_BLOCKS_TOTAL)
        return -SSD_ERR_NO_INVALID_BLOCKS;
    /* pos_in_channel->[phy_block]               */
    /* 0->[0], 1->[1],  2->[4096],  3->[4097]    */
    /* 4->[2], 5->[3],  6->[4098],  7->[4099]    */
    /* 8->[4], 9->[5], 10->[4100], 11->[4101]    */
    /* ....                                      */ 
    pos_in_channel = block_pos % RESERVE_BLOCKS_PER_CHANNEL;
    phyblock = ((pos_in_channel >> 2) << 1) + plane_table_start[pos_in_channel % SSD_MAX_PLANES];
    
    return phyblock;
}

static inline int write_one_reserve_block(struct ssd_device *sdev, ssd_u16 block_pos, char* buffer)
{
    int ret;
    ssd_u64 cmd;
    int page;
    ssd_u8 channel;
    ssd_u16 phy_block;
    struct semaphore *sema;
    
    /* calculate phyblock and channel */
    ret = calc_phyblock(block_pos);
    if (unlikely(ret < 0))
        return ret;
    phy_block = (ssd_u16)ret;
    channel = block_pos / RESERVE_BLOCKS_PER_CHANNEL;
    sema = &(sdev->channel_ctl[channel].sema);
    
    /* write one block */
    //cmd = cmd_value(phy_block * SSD_PAGES_PER_BLOCK, SSD_PAGES_PER_BLOCK, SSD_FLAG_WRITETABLE_TO_FLASH);
    down(sema);
    //memcpy(sdev->kbuftable_cpu[channel], buffer, SSD_BLOCK_SIZE);
    for (page = 0; page < SSD_PAGES_PER_BLOCK; page++) {
        memcpy(sdev->kbuftable_cpu[channel], buffer + page *SSD_PAGE_SIZE, SSD_PAGE_SIZE);
        cmd = cmd_value(phy_block * SSD_PAGES_PER_BLOCK + page, 1, SSD_FLAG_WRITETABLE_TO_FLASH);
        ret = channel_op(sdev, channel, cmd);
        if (ret != SSD_OK)
            break;
    }
    up(sema);
    
    return ret;
}


static inline int read_one_reserve_block(struct ssd_device *sdev, ssd_u16 block_pos)
{
    int ret;
    ssd_u64 cmd;
    int page;
    ssd_u8 channel;
    ssd_u16 phy_block;
    struct semaphore *sema;
    
    /* calculate phyblock and channel */
    ret = calc_phyblock(block_pos);
    if (unlikely(ret < 0))
        return ret;
    phy_block = (ssd_u16)ret;
    channel = block_pos / RESERVE_BLOCKS_PER_CHANNEL;
    sema = &(sdev->channel_ctl[channel].sema);

    down(sema);

    for (page = 0; page < SSD_PAGES_PER_BLOCK; page++) {
        cmd = cmd_value(phy_block * SSD_PAGES_PER_BLOCK + page, 1, SSD_FLAG_READTABLES_FROM_FLASH);
        ret = channel_op(sdev, channel, cmd);
        if (ret != SSD_OK)
            break;
    }
    up(sema);
    
    return ret;
}

static int erase_one_reserve_block(struct ssd_device *sdev, ssd_u16 block_pos)
{
    int ret;
    ssd_u64 cmd;
    ssd_u8 channel;
    ssd_u16 phy_block;
    struct semaphore *sema;
    
    /* calculate phyblock and channel */
    ret = calc_phyblock(block_pos);
    if (unlikely(ret < 0))
        return ret;
    phy_block = (ssd_u16)ret;
    channel = block_pos / RESERVE_BLOCKS_PER_CHANNEL;
    sema = &(sdev->channel_ctl[channel].sema);
        
    /* erase one block */
    cmd = cmd_value(phy_block * SSD_PAGES_PER_BLOCK, 1, SSD_FLAG_ERASETABLES);
    down(sema);
    ret = channel_op(sdev, channel, cmd);
    up(sema);
    
    return ret;
}

/* write one block with duplicate; if failure, set table_bad_bitmap */
static int write_tabledata_block_duplicate(struct ssd_device *sdev, ssd_u16 cur_pos, char *buffer)
{
    int ret;
    int i;
    ssd_u16 write_pos;

    for (i = 0; i < TABLE_DUPLICATE; i++) {
        write_pos = cur_pos + i * RESERVE_BLOCKS_EACH_TABLE;
        /* bitmap for tabledata start from channel 2 */
        
        /* erase block */
        ret = erase_one_reserve_block(sdev, write_pos);
        if (unlikely(ret != SSD_OK)) {
            return ret;
        }
        /* write block */
        ret = write_one_reserve_block(sdev, write_pos, buffer);
        if (unlikely(ret != SSD_OK))
            return ret;
            
        ret = read_one_reserve_block(sdev, write_pos);
        if (unlikely(ret != SSD_OK)) {
            ssd_log(SSD_LOG_WARNING, "write tabledata block, but read error, block_pos: %u, ret = %d\n", write_pos, ret);
            return ret;
        }
    }
    
    return SSD_OK;
}


/* update some members of metadata */
static inline void update_meta(struct ssd_device *sdev, ssd_u16 block_num, ssd_u16 last_block_used_bytes, int flush_type, ssd_u32 id_num)
{
    struct timex txc;
    
    if (flush_type == FLUSH_NORMAL_TYPE)
        sdev->meta.metadata_magic = FLUSH_MAGIC_NORMAL;
    else
        sdev->meta.metadata_magic = FLUSH_MAGIC_CLOSE;
        
    sdev->meta.block_num = block_num;
    sdev->meta.last_block_used_bytes = last_block_used_bytes;
    sdev->meta.metadata_crc32 = 0;
    sdev->meta.id_num = id_num;
    sdev->meta.version = sdev->meta.version + 1;
    if (sdev->meta.version == 0)
        sdev->meta.version = 1;

    do_gettimeofday(&(txc.time));
    sdev->meta.timestamp = txc.time.tv_sec;
}

static inline void set_table_pos_in_meta(struct ssd_device *sdev, ssd_u16 block_num, ssd_u16 cur_pos)
{
    int i;
    
    /* block_num|   pos of one block      |             pos of duplicate             */
    /* 0        | table_block_pos[0]= 200 | table_block_pos[1]=21channel*20block+200 */
    /* 1        | table_block_pos[2]= 201 | table_block_pos[3]=21channel*20block+201 */
    /* 2        | table_block_pos[4]= 203 | table_block_pos[5]=21channel*20block+203 */
    for (i = 0; i < TABLE_DUPLICATE; i++)
         sdev->meta.table_block_pos[block_num * TABLE_DUPLICATE + i] 
            = cur_pos + i * SSD_MAX_CHANNEL_EACH_TABLE * RESERVE_BLOCKS_PER_CHANNEL;
}

static ssd_s32 retry_write_tabledata_block(struct ssd_device *sdev, ssd_u16 block_num, ssd_u16 last_pos, char *buffer)
{
    int ret;
    int err_num;
    ssd_u16 cur_pos = last_pos;
    
    for (err_num = 0; err_num < SSD_MAX_FLUSH_TABLE_RETRY; err_num++) {
        /* prepare current persistence block position */    
        ret = find_new_table_pos(sdev, cur_pos);
        if (unlikely(ret < 0))
            return -SSD_ERR_NO_VALID_RESERVE;
        cur_pos = (ssd_u16)ret;
    
        /* write one block with duplicate */
        ret = write_tabledata_block_duplicate(sdev, cur_pos, buffer);
        if (likely(ret == SSD_OK)) {
            set_table_pos_in_meta(sdev, block_num, cur_pos);
            return cur_pos;
        }
    }

    return SSD_ERROR;
}

/* step 3 of flush process: flush tabledata to flash */ 
static int flush_tabledata(struct ssd_device *sdev, char *buffer, ssd_u32 id_num, int flush_type)
{
    int i;
    int ret;
    ssd_u16 cur_pos;
    char *p = buffer;
    ssd_u32 tabledata_bytes;
    ssd_u16 block_num;
    ssd_u32 last_block_used_bytes;

    if (id_num > SSD_MAX_ID_NUM)
        return SSD_ERROR;
    /* calculate block_num and last_block_used_bytes */   
    tabledata_bytes = SSD_TABLE_SIZE * SSD_TABLE_TYPE_NUM * SSD_MAX_CHANNEL 
                      + id_num * sizeof(struct flush_id_info);
    block_num = tabledata_bytes >> SSD_BLOCK_SHIFT;
    /* 4.0M -> block_num = 2, last_block_used_bytes = 2M   */
    /* 4.1M -> block_num = 3, last_block_used_bytes = 0.1M */
    last_block_used_bytes = tabledata_bytes - block_num * SSD_BLOCK_SIZE;
    if (last_block_used_bytes == 0)
        last_block_used_bytes = SSD_BLOCK_SIZE;
    else
        block_num++;

    /* find last persistence block position */
    cur_pos = sdev->meta.table_block_pos[TABLE_DUPLICATE * sdev->meta.block_num - TABLE_DUPLICATE];
    /* write persistent tabledata block to flash */    
    for (i = 0; i < block_num; i++, p += SSD_BLOCK_SIZE) {
        ret = retry_write_tabledata_block(sdev, i, cur_pos, p);
        if (ret < 0)
            return ret;
        cur_pos = (ssd_u16)ret;
    }
    update_meta(sdev, block_num, last_block_used_bytes, flush_type, id_num);
    
    return SSD_OK;
}

static int write_one_reserve_page(struct ssd_device *sdev, ssd_u16 block_pos, ssd_u8 page)
{
    int ret;
    ssd_u64 cmd;
    ssd_u8 channel;
    ssd_u16 phy_block;
    struct semaphore *sema;

    /* calculate phyblock and channel */
    ret = calc_phyblock(block_pos);
    if (unlikely(ret < 0))
        return ret;
    phy_block = (ssd_u16)ret;
    channel = block_pos / RESERVE_BLOCKS_PER_CHANNEL;
    sema = &(sdev->channel_ctl[channel].sema);

    /* write one page */
    cmd = cmd_value(phy_block * SSD_PAGES_PER_BLOCK + page, 1, SSD_FLAG_WRITETABLE_TO_FLASH);

    down(sema);
    memset(sdev->kbuftable_cpu[channel], 0, SSD_PAGE_SIZE); /*metadata cannot fill one page */
    memcpy(sdev->kbuftable_cpu[channel], &(sdev->meta), sizeof(struct ssd_metadata));
    ret = channel_op(sdev, channel, cmd);
    up(sema);
    
    return ret;
}

static int read_one_reserve_page(struct ssd_device *sdev, ssd_u16 block_pos, ssd_u8 page)
{
    int ret;
    ssd_u64 cmd;
    ssd_u8 channel;
    ssd_u16 phy_block;
    struct semaphore *sema;

    /* calculate phyblock and channel */
    ret = calc_phyblock(block_pos);
    if (unlikely(ret < 0))
        return ret;
    phy_block = (ssd_u16)ret;
    channel = block_pos / RESERVE_BLOCKS_PER_CHANNEL;
    sema = &(sdev->channel_ctl[channel].sema);

    /* write one page */
    cmd = cmd_value(phy_block * SSD_PAGES_PER_BLOCK + page, 1, SSD_FLAG_READTABLES_FROM_FLASH);

    down(sema);
    ret = channel_op(sdev, channel, cmd);
    up(sema);
    
    return ret;
}

static int write_meta_page(struct ssd_device *sdev, int plane_num)
{
    int ret;
    ssd_u8 page = sdev->meta_pos.page;
    ssd_u16 block_pos = sdev->meta_pos.block_pos;
  
    /* check parameter */
    if (block_pos % SSD_MAX_PLANES != 0)
        return -SSD_ERR_META_BLOCK_ALIGN;
    block_pos += plane_num;

    /* if page == 0, erase block first */
    if (page == 0) {
        ret = erase_one_reserve_block(sdev, block_pos);
        if (unlikely(ret != SSD_OK))
            return ret;
    }
        
    /* write page */
    ret = write_one_reserve_page(sdev, block_pos, page);
    if (ret != SSD_OK)
        return ret;
 
    ret = read_one_reserve_page(sdev, block_pos, page);
    if(ret != SSD_OK)
        ssd_log(SSD_LOG_WARNING, "write metadata page, but read error, block_pos: %u, page: %u, ret = %d\n",block_pos, page, ret);
    
    printk("write meta page block=%d, page=%d, ret=%d\n",block_pos,page,ret);
    return ret;
}

static int write_metadata_page_duplicate(struct ssd_device *sdev)
{
    int ret;
    int plane_num;
    int duplicate_success = 0;
    
    for (plane_num = 0; plane_num < SSD_MAX_PLANES; plane_num++) {
        ret = write_meta_page(sdev, plane_num);
        if (likely(ret == SSD_OK))
            duplicate_success++;
    }
    if (duplicate_success >= SSD_MIN_META_DUPLICATE)
        return SSD_OK;
    else
        return SSD_ERROR;   
}

static inline ssd_s32 find_new_meta_block(struct ssd_device *sdev, ssd_u16 cur_pos)
{
    ssd_u16 bitmap_pos;

    /* each 4 plane -> 1 bitmap */
    bitmap_pos = cur_pos / SSD_MAX_PLANES;
    bitmap_pos = (bitmap_pos + 1) % META_BITMAP_SIZE;
    return (bitmap_pos * SSD_MAX_PLANES);
}

/* set next page to write, if last operation is fail, change to next block first block*/
static inline int find_new_meta_pos(struct ssd_device *sdev, int last_success)
{
    int ret;
    struct meta_position *cur_meta_pos;
   
    cur_meta_pos = &(sdev->meta_pos);
    if (unlikely(last_success != SSD_OK)) {
        /* set meta pos */
        cur_meta_pos->page = 0;
        ret = find_new_meta_block(sdev, cur_meta_pos->block_pos);
        if(ret < 0)
            return ret;
        cur_meta_pos->block_pos = (ssd_u16)ret;
    }
    else {
        /* set meta pos */
        if (unlikely(cur_meta_pos->page == SSD_PAGES_PER_BLOCK - 1)) {
            cur_meta_pos->page = 0;
            ret = find_new_meta_block(sdev, cur_meta_pos->block_pos);
            if(ret < 0)
                return ret;
            cur_meta_pos->block_pos = (ssd_u16)ret;
        }
        else
            (cur_meta_pos->page)++;
    }
    
    return SSD_OK;
}

/* step 4 of flush process: flush tabledata to flash */ 
static int flush_metadata(struct ssd_device *sdev)
{    
    int ret;
    int err_num;
    
    ret = SSD_OK;
    for (err_num = 0; err_num < SSD_MAX_FLUSH_META_RETRY; err_num++) {
        /* parpare next page to write */
        ret = find_new_meta_pos(sdev, ret);
        if (unlikely(ret != SSD_OK))
            return ret;
        /* write one page with 4 duplicate */
        ret = write_metadata_page_duplicate(sdev);
        if (likely(ret == SSD_OK))
            break;
    }
    
    return ret;
}

static int ssd_sys_flush(struct ssd_device *sdev, int flush_type)
{
    int ret;
    ssd_u8 channel;
    char *buffer_head, *buffer;
    ssd_u32 id_num;

    if (g_opened_flag != SSD_OPENED)
        return SSD_OK;
        
    down(&flush_sema);
    buffer_head = sdev->kbuf_flush;
    memset(buffer_head, 0, SSD_MAX_FLUSH_SIZE);
    
    /* step 1: read erase and addr table from each channel in ssd */
    buffer = buffer_head;
    for (channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
        ret = copy_ram_table(sdev, channel, buffer);
        if (unlikely(ret != SSD_OK)) {
            ssd_log(SSD_LOG_ERR, "read erase and addr table failed in channel[%u], flush_type = %d, ret = %d (line %d)\n", channel, flush_type, ret, __LINE__);
            goto out;
        }
        buffer += SSD_TABLE_SIZE * SSD_TABLE_TYPE_NUM;
    }
    
    /* step 2: copy id hash table */
    id_num = copy_id_hash(sdev, (struct flush_id_info *)buffer);
    ssd_log(SSD_LOG_INFO, "flush id_num is %u\n", id_num);
    
    /* step 3: flush all the table data to ssd and update meta-data in memory */
    ret = flush_tabledata(sdev, buffer_head, id_num, flush_type);
    if (unlikely(ret != SSD_OK)) {
        ssd_log(SSD_LOG_ERR, "flush table data failed, flush_type = %d, ret = %d (line %d)\n", flush_type, ret, __LINE__);
        goto out;   
    }    
    /* step 4: flush meta-data to ssd */
    ret = flush_metadata(sdev);
    if (unlikely(ret != SSD_OK)) {
        ssd_log(SSD_LOG_ERR, "flush meta data failed, flush_type = %d, ret = %d (line %d)\n", flush_type, ret, __LINE__);
    }    
    
out:
    up(&flush_sema);
    return ret;
}


static int ssd_read_idtable(struct ssd_device *sdev, struct ssd_readid_param *ridparam)
{
    int ret;
    ssd_u8 channel;
    int hash_list;
    ssd_u32 id_num = 0;
    ssd_u32 id_num_in_channel;
    ssd_u8 *usr_buf = ridparam->usr_buf;
    struct ssd_hash_node *node;
    struct ssd_hash_head *head;
    struct channel_hash_list *ch_hash;
    struct flush_id_info *buffer_head, *buffer;
    
    //only a buffer for one channel
    buffer_head = vmalloc(SSD_MAX_ID_NUM_PER_CHANNEL * sizeof(struct flush_id_info));
    if (!buffer_head) {
        ssd_log(SSD_LOG_ERR, "memory alloc fail of ssd_device (line %d)\n", __LINE__);
        ret = -ENOMEM;
        goto fail;
    }
    memset(buffer_head, 0, SSD_MAX_ID_NUM_PER_CHANNEL * sizeof(struct flush_id_info));
    
    for (channel = 0; channel < SSD_MAX_CHANNEL; channel++) { 
        ch_hash = &(sdev->ch_hash[channel]);
        buffer = buffer_head;
        id_num_in_channel = 0;
        for (hash_list = 0; hash_list < SSD_HASH_SIZE; hash_list++) {
            head = &(ch_hash->head[hash_list]);
            
            spin_lock(&(head->hash_lock));
            node = head->first;
            while (node) {
                prefetch(node->next);
                copy_hash_node_to_flush_id_info(node, buffer);
                buffer++;
                id_num_in_channel++;
                id_num++;
                node = node->next;
            }          
            spin_unlock(&(head->hash_lock));
        }
        if (id_num_in_channel > 0) {
            if (copy_to_user(usr_buf, buffer_head, id_num_in_channel * sizeof(struct flush_id_info))) {
                ssd_log(SSD_LOG_ERR, "ssd_read_idtable: copy to user failed (line %d)\n", __LINE__);
                ret = -SSD_ERR_COPY_TO_USER;
                goto out;
            }
        }
        usr_buf += id_num_in_channel * sizeof(struct flush_id_info);
    }
    ret = id_num;

out:
    vfree(buffer_head);
fail:
    if (ret < SSD_OK)
        ret = SSD_MAX_ID_NUM + 1;
    return ret;
}

// check the metadata to see whether it is valid or not
static inline int metadata_check_val(struct ssd_metadata *metadata)
{
    if (metadata->metadata_magic != FLUSH_MAGIC_NORMAL && metadata->metadata_magic != FLUSH_MAGIC_CLOSE)
        return -EMETAUNVAL;

    // CRC32 check

    return SSD_OK;
}

static inline int ssd_read_metadata(struct ssd_device *sdev, ssd_u16 meta_block, ssd_u8 meta_page)
{
    int      ret;
    ssd_u64  cmd;
    ssd_u8   channel;
    ssd_u16  phy_block;
    ssd_u32  start;
    
    channel = meta_block / RESERVE_BLOCKS_PER_CHANNEL; 
    phy_block = calc_phyblock(meta_block);
    start = phy_block * SSD_PAGES_PER_BLOCK + meta_page;
    cmd = cmd_value(start, 1, SSD_FLAG_READTABLES_FROM_FLASH);

    ret = channel_op(sdev, channel, cmd);
    return ret;
}

static inline int ssd_read_tabledata(struct ssd_device *sdev, ssd_u16 tabledata_block, ssd_u8 page)
{
    int      ret;
    ssd_u64  cmd;
    ssd_u8   channel;
    ssd_u16  phy_block;
    ssd_u32  start;

    channel = tabledata_block / RESERVE_BLOCKS_PER_CHANNEL; 
    phy_block = calc_phyblock(tabledata_block);
    start = phy_block * SSD_PAGES_PER_BLOCK;
    //cmd = cmd_value(start, SSD_PAGES_PER_BLOCK, SSD_FLAG_READTABLES_FROM_FLASH);
    cmd = cmd_value(start+page, 1, SSD_FLAG_READTABLES_FROM_FLASH);

    ret = channel_op(sdev, channel, cmd);
    return ret;
}


static int search_block_metadata(struct ssd_device *sdev)
{
    int      ret;
    int      plane_num;
    int      block_pos;
    ssd_u16  meta_block;
    ssd_u64  meta_version; 
    ssd_u8   *kbuf;
    ssd_u8   channel;
    ssd_u8   meta_val;

    struct ssd_metadata *metadata;

    //init meta version
    meta_version = 0;
    for (block_pos = 0; block_pos < RESERVE_BLOCKS_META / SSD_MAX_PLANES; block_pos++) {

        // read four plane until get one valid metadata
        for (plane_num = 0; plane_num < SSD_MAX_PLANES; plane_num++) {    
            meta_block = block_pos * SSD_MAX_PLANES + plane_num;
            channel = meta_block / RESERVE_BLOCKS_PER_CHANNEL; 
            
            ret = ssd_read_metadata(sdev, meta_block, 0);
            //cur_plane has no valid metadata, read next plane 
            if (ret == -SSD_ERR_TIMEOUT)
                return ret;
            if (ret != SSD_OK) 
                continue;
            

            kbuf = sdev->kbuftable_cpu[channel];
            metadata = (struct ssd_metadata *) kbuf;
            // check whether the metadata is valid or not
            meta_val = metadata_check_val(metadata);
            if (meta_val != SSD_OK)
                continue;

            //get valid metadata
            if (metadata->version > meta_version) {
                meta_version = metadata->version;
                sdev->meta_pos.block_pos = block_pos * SSD_MAX_PLANES;
            }
            break; 

        }
    }


    if (likely(meta_version != 0)) {
        //ssd_log(SSD_LOG_ERR, "SSD: faild in search metadata!\n");
        return SSD_OK;
    }

    return -ENOMETA;

}

static int search_page_metadata(struct ssd_device *sdev)
{
    int      ret;
    int      plane_num;
    int      page_num;
    ssd_u8   *kbuf;
    ssd_u8   channel;
    ssd_u8   meta_val;
    ssd_u8   block_pos;
    ssd_u16  meta_cur_block;
    
    struct ssd_metadata *metadata;

    block_pos = sdev->meta_pos.block_pos;
    channel = block_pos / RESERVE_BLOCKS_PER_CHANNEL; 

    for (page_num = SSD_PAGES_PER_BLOCK -1; page_num >= 0; page_num--) {
        for (plane_num = 0; plane_num < SSD_MAX_PLANES; plane_num++) {
            meta_cur_block = block_pos + plane_num;
            ret = ssd_read_metadata(sdev, meta_cur_block, page_num);
            //cur_plane has no valid metadata, read next plane 
            if (ret == -SSD_ERR_TIMEOUT)
                return ret;
            if (ret != SSD_OK)
                continue;

            kbuf = sdev->kbuftable_cpu[channel];
            metadata = (struct ssd_metadata *) kbuf;
            // check whether the metadata is valid or not
            meta_val = metadata_check_val(metadata);
            if (meta_val != SSD_OK) 
                continue;

            //get valid metadata
            sdev->meta = *metadata;
            sdev->meta_pos.page = (ssd_u8)page_num;
            return SSD_OK;
        }
    }

    return -EREADFLUSH;
}


static int ssd_search_metadata(struct ssd_device *sdev)
{
    int  ret;

    ret = search_block_metadata(sdev);

    // can not find valid metadata
    if (ret != SSD_OK)
        return ret;
    return search_page_metadata(sdev);
}

static int ssd_read_flush_tabledata(struct ssd_device *sdev, char *buffer)
{
    int      ret, i;
    ssd_u32  page;
    int      block_num;
    ssd_u8   channel;
    ssd_u16  tabledata_block;

    for (block_num = 0; block_num < sdev->meta.block_num; block_num++) {
        for (i = 0; i < TABLE_DUPLICATE; i++) {
            tabledata_block = sdev->meta.table_block_pos[block_num * TABLE_DUPLICATE + i];
            channel = tabledata_block / RESERVE_BLOCKS_PER_CHANNEL;
            
            // ret = ssd_read_tabledata(sdev, tabledata_block);
            // if (likely(ret == SSD_OK)) {
                // memcpy(buffer + SSD_BLOCK_SIZE * block_num, sdev->kbuftable_cpu[channel], SSD_BLOCK_SIZE);
                // break;
            //}
            for (page = 0; page < SSD_PAGES_PER_BLOCK; page++) {
                ret = ssd_read_tabledata(sdev, tabledata_block, page);
                if (ret == -SSD_ERR_TIMEOUT)
                    return ret;
                if (ret != SSD_OK)
                    break;
                memcpy(buffer + SSD_BLOCK_SIZE * block_num + SSD_PAGE_SIZE * page, sdev->kbuftable_cpu[channel], SSD_PAGE_SIZE);
                
            }
            if (page == SSD_PAGES_PER_BLOCK) /*read one block success*/
                break;
        }
        // can not read one valid DUPLICATE of this block
        if (unlikely(i == TABLE_DUPLICATE)) {
            ssd_log(SSD_LOG_ERR, "failed to read tabledata! (line %d)\n", __LINE__);
            return -ETABLEDATA;
        }

    }

    return SSD_OK;    
}

//read certain version of flush tabledata
static int ssd_read_flush(struct ssd_device *sdev, struct ssd_meta_version *version)
{
    int ret;
    ssd_u8 *buff;
    ssd_u8 *kbuf;
    int meta_val;
    struct semaphore *sema;
    struct ssd_metadata *metadata;
    int channel = version->block / RESERVE_BLOCKS_PER_CHANNEL; 

    if (version->block >= RESERVE_BLOCKS_META)
        return -SSD_ERR_INVALID_PARAM;
    
    sema = &sdev->channel_ctl[channel].sema;
    down(sema);
    
    //read metadata
    ret = ssd_read_metadata(sdev, version->block, version->page);
    if (SSD_OK != ret) {
        ssd_log(SSD_LOG_ERR, "SSD:  failed in read metadata, error :%d! (line %d)\n", ret, __LINE__);
        up(sema);
        return ret;
    }
    kbuf = sdev->kbuftable_cpu[channel];
    metadata = (struct ssd_metadata *) kbuf;
    // check whether the metadata is valid or not
    meta_val = metadata_check_val(metadata);
    if (SSD_OK != meta_val) {
        up(sema);
        return EMETAUNVAL;
    }
    //update sdev info 
    sdev->meta_pos.block_pos = version->block;
    sdev->meta = *metadata;
    sdev->meta_pos.page = version->page;
    sdev->meta.metadata_magic = FLUSH_MAGIC_CLOSE;

    //read table data based on metadata and buffer it in kbuf_flush
    buff = sdev->kbuf_flush;
    ret = ssd_read_flush_tabledata(sdev, buff);
    if (unlikely(ret != SSD_OK)) {
        ssd_log(SSD_LOG_ERR, "init failed in read flush tabledata, error :%d! (line %d)\n", ret, __LINE__);
        goto OUT;
    }
    
    //copy the flush data to buffer in user mode
    if (copy_to_user(version->buff, buff, sdev->meta.block_num * SSD_BLOCK_SIZE)) {
        ssd_log(SSD_LOG_ERR, "ssd_read_flush: copy to user failed (line %d)\n", __LINE__);
        ret = -SSD_ERR_COPY_TO_USER;
        goto OUT;
    }
    

OUT:
    up(sema);
    return ret;
}



static int ssd_id_val(struct flush_id_info *id, struct addr_entry *addr, struct erase_entry *erase)
{
    ssd_u16   len = id->len ? BT_DATA_BLOCK_NUM : BT_INDEX_BLOCK_NUM;
    ssd_u16   start, logic_block, phy_block;

    start = (ssd_u16)id->block; 
    
    for (logic_block = start; logic_block < start + len; logic_block++) {
        phy_block = (addr + logic_block)->pb_addr;
        
        if (!(erase + phy_block)->use_flag || (erase + phy_block)->bad_flag)   //unused
            return -EIDUNVAL;

    }

    // crc32 check

    return SSD_OK;
}

static int ssd_rebuild_hash(struct ssd_device *sdev, char *buffer)
{
    int       ret = SSD_OK;
    ssd_u8    channel; 
    int       id_num, id_max;
    int       normal_uninstall;
    int       ret_id;

    struct addr_entry    *addr_buf, *addr;
    struct erase_entry   *erase_buf, *erase;
    struct flush_id_info *id_buf;
    struct ssd_blockid   id_tmp;
    struct id_info       id_info_tmp;

    normal_uninstall = sdev->meta.metadata_magic == FLUSH_MAGIC_CLOSE;
    addr_buf = (struct addr_entry *)buffer;
    erase_buf = (struct erase_entry *)(buffer + SSD_TABLE_SIZE);
    id_buf = (struct flush_id_info *)(buffer + SSD_MAX_CHANNEL * SSD_TABLE_SIZE * SSD_TABLE_TYPE_NUM);
    id_max = sdev->meta.id_num;


    for (id_num = 0; id_num < id_max; id_num++, id_buf++) {       
        channel = (ssd_u8)id2channel(&id_buf->id);
        if(channel != id_buf->channel) {
            ssd_log(SSD_LOG_WARNING, "channel[%u] not match id_buf[%u]\n", channel, id_buf->channel);
            /*fixed channel for flush_id_buf*/
            id_buf->channel = channel;
        }
        
        if (channel >= SSD_MAX_CHANNEL) {
            ssd_log(SSD_LOG_WARNING, "channel of the id in tabledata is out of bounds!\n");
            continue;
        }

        if (!normal_uninstall) {
            addr = addr_buf + channel * SSD_TABLE_SIZE * SSD_TABLE_TYPE_NUM / sizeof(struct addr_entry);
            erase = erase_buf + channel * SSD_TABLE_SIZE * SSD_TABLE_TYPE_NUM / sizeof(struct erase_entry);
            ret = ssd_id_val(id_buf, addr, erase);
            // unvalid id
            if (ret != SSD_OK) {
                ssd_log(SSD_LOG_WARNING, "erase invalid id: %llx%llx (line %d)\n", id_buf->id.m_nHigh, id_buf->id.m_nLow, __LINE__);
                continue;
            }
        }

        id_tmp.m_nLow = id_buf->id.m_nLow;
        id_tmp.m_nHigh = id_buf->id.m_nHigh;
        id_info_tmp.channel = id_buf->channel;
        id_info_tmp.block = id_buf->block;
        id_info_tmp.len = id_buf->len;

        ret_id = set_id_info(sdev, &id_tmp, &id_info_tmp);

        if (unlikely(ret_id != SSD_OK))
            return ret_id;
    }

    return SSD_OK;
}

static inline int ssd_mark_bad_block(struct ssd_device *sdev, struct ssd_mark_bad_param *mbparam)
{
    ssd_u32 len, start;
    ssd_u8 channel;
    ssd_u16 *blocks = NULL, block;
    ssd_u16 phy_block;
    struct ssd_table_param stparam;
    struct addr_entry *addr = NULL;
    struct erase_entry *erase = NULL;
    int ret = SSD_OK;
    ssd_u64 cmd;
    id_info_t id_info;
    struct semaphore *sema;
    
    switch (mbparam->type) {
        case PARAM_ID:
            /*get channel and block*/
            channel = (ssd_u8)id2channel(&mbparam->u.id);
        
            memset(&id_info, 0, sizeof(id_info_t));
            sema = &sdev->channel_ctl[channel].sema;            
            down(sema);
            
            ret = get_id_info(sdev, &mbparam->u.id, &id_info);
            if (ret != SSD_OK) {
                ssd_log(SSD_LOG_ERR, "ssd_id_read: get_id_info failed, ret[%d], channel=%u, id.High=%llx id.Low=%llx (line %d)\n",
                    ret, channel, mbparam->u.id.m_nHigh, mbparam->u.id.m_nLow, __LINE__);
                ret = -SSD_ID_NO_EXIST;
                goto out;
            }

            len = BLOCKSIZE[id_info.len];
            block = id_info.block;
            
            blocks = kmalloc(len * sizeof(ssd_u16), GFP_KERNEL);
            if (!blocks) {
                ssd_log(SSD_LOG_ERR, "memory alloc fail of ssd_device (line %d)\n", __LINE__);
                ret = -ENOMEM;
                goto out;
            }
    //        memset(blocks, 0, len * sizeof(ssd_u32));

            /*check channel*/
            if (unlikely(channel != id_info.channel)) {
                ssd_log(SSD_LOG_ERR, "channel[%u] not match id_info[%u] (%d)\n", channel, id_info.channel, __LINE__);
                ret = SSD_ERROR;
                goto out;                
            }
            
            for (start = 0; start < len; start++) {
                cmd = cmd_value((block + start) * SSD_PAGES_PER_BLOCK , SSD_PAGES_PER_BLOCK, SSD_FLAG_READ);
                ret = channel_op(sdev, channel, cmd);

                /*find it*/
                if (ret != SSD_OK)
                    blocks[start] = BAD_BLOCK;                    
                else
                    blocks[start] = INVALID_VALUE;
            }    

            /*check for bad block*/
            for (start = 0; start < len; start++)
                if (BAD_BLOCK == blocks[start])
                    break;

            if (start == len) {
                ssd_log(SSD_LOG_ERR, "there's no bad block in %llx%llx (%d)\n", 
                    mbparam->u.id.m_nHigh, mbparam->u.id.m_nLow, __LINE__);
                ret = SSD_ERR_NOT_BAD_BLOCK;
                goto out;
            }

            /*delete ID*/
            ret = del_id_info(sdev, &mbparam->u.id, &id_info);
            if(ret != SSD_OK) {
                ssd_log(SSD_LOG_ERR, "ssd_id_delete: del_id_info failed, ret[%d], id.High=%llx id.Low=%llx channel=%u block=%u len=%u (line %d)\n",
                    ret, mbparam->u.id.m_nHigh, mbparam->u.id.m_nLow, id_info.channel, id_info.block, id_info.len, __LINE__);
                ret = -SSD_ID_NO_EXIST;
                goto out;
            }

            cmd = cmd_value(block, len, SSD_FLAG_ERASE);
            ret = channel_op(sdev, channel, cmd);

            if (unlikely(ret != SSD_OK)) {
                ssd_log(SSD_LOG_ERR, "erase block failed: ret[%d], id.High=%llx id.Low=%llx channel=%u block=%u len=%u (line %d)\n", 
                    ret, mbparam->u.id.m_nHigh, mbparam->u.id.m_nLow, id_info.channel, id_info.block, id_info.len, __LINE__);
                if(SSD_OK != set_id_info(sdev, &mbparam->u.id, &id_info))
                    ssd_log(SSD_LOG_ERR, "redo set id info error\n");

                goto out;
            }

            delete_logic_block(sdev, &id_info);

            break;

        case PARAM_BLOCK:
            block = mbparam->u.param_block.block;
            channel = (ssd_u8)mbparam->u.param_block.channel;
            len = BLOCKSIZE[IBLOCK];
            
            sema = &sdev->channel_ctl[channel].sema;            
            down(sema);

            blocks = kmalloc(len * sizeof(ssd_u16), GFP_KERNEL);
            if (!blocks) {
                ssd_log(SSD_LOG_ERR, "memory alloc fail of ssd_device (line %d)\n", __LINE__);
                ret = -ENOMEM;
                goto out;
            }

            blocks[0] = BAD_BLOCK;
            
            break;
            
        default:
            ssd_log(SSD_LOG_ERR, "ssd_mark_bad_block type error[%u] (line %d)\n", 
                mbparam->type, __LINE__);
            return -SSD_ERR_INVALID_PARAM;
    }
    
    /*get addr table for physical addr*/
    stparam.usr_buf = NULL;
    stparam.channel = channel;
    stparam.type = TABLE_ADDR_TYPE;

    ret = ram_table_read(sdev, &stparam);
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "fail to read addr ram table, channel = %u type = %u ,ret[%d] (line %d)\n", 
            stparam.channel, stparam.type, ret, __LINE__);
        goto out;
    }

    addr = (struct addr_entry *)sdev->kbuftable_cpu[channel];

    for (start = 0; start < len; start++) {
        if(BAD_BLOCK == blocks[start]) {
            phy_block = (addr + block + start)->pb_addr;
            ssd_log(SSD_LOG_WARNING, "get physical bad block %u, logic block[%u] in channel[%u] (%d)\n", phy_block, block + start, channel, __LINE__);
            blocks[start] = phy_block;
        }
    }

    /*get erase table for mark*/
    stparam.usr_buf = NULL;
    stparam.channel = channel;
    stparam.type = TABLE_ERASE_TYPE;

    ret = ram_table_read(sdev, &stparam);
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "fail to read erase ram table, channel = %u type = %u ,ret[%d] (line %d)\n", 
            stparam.channel, stparam.type, ret, __LINE__);
        goto out;
    }

    erase = (struct erase_entry *)sdev->kbuftable_cpu[channel];
    for (start = 0; start < len; start++) {
        if (INVALID_VALUE != blocks[start])
            (erase + blocks[start])->bad_flag = 1;
    }

    /*copy to fpga ram*/
    
    ret = ssd_write_tables_to_ram(sdev, channel, SSD_FLAG_WRITE_ERASE_TABLE);
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "init failed in writing erase table to ram, error: %d, channel[%u] (line %d)\n", ret, channel, __LINE__); 
        goto out;
    }

    /*flush table*/
//    ret = ssd_sys_flush(sdev, FLUSH_NORMAL_TYPE);

out:
    if (blocks)
        kfree(blocks);
    up(sema);
    return ret;
}

static inline int ssd_mark_erase_count(struct ssd_device *sdev, struct ssd_mark_erase_count_param *mecparam)
{
    int ret = SSD_OK;
    ssd_u16 i, phy_block;
    ssd_u16 channel, count;
    ssd_u8 *map = NULL;
    struct erase_entry_map *eem;
    struct erase_entry *erase_table;
    struct ssd_table_param stparam;   
    struct semaphore *sema;

    /*******************************
    *|eem|eem|eem|...          |eem|
    ********************************
    *   /     \
    *  /       \
    * |block|ee|
    */
    channel = mecparam->channel;
    count = mecparam->count;    

    if(channel >= SSD_MAX_CHANNEL) {
        ssd_log(SSD_LOG_ERR, "channel[%u] is error (line %d)\n", channel, __LINE__);
        ret = SSD_ERROR;
        goto fail;
    }

    if (count > SSD_MAX_PHYBLOCKS_PER_CHANNEL) {
        ssd_log(SSD_LOG_ERR, "count[%u] is not match erase_buffer, channel[%u] (line %d)\n", count, channel, __LINE__);
        ret = SSD_ERROR;
        goto fail;
    }

    /*erase_buffer size = count * sizeof(erase_entry_map)*/
    map = vmalloc(count * sizeof(struct erase_entry_map));
    if (!map) {
        ssd_log(SSD_LOG_ERR, "memory alloc fail of ssd_device (line %d)\n", __LINE__);
        ret = -ENOMEM;
        goto fail;
    }
//    memset(map, 0, count * sizeof(ssd_u16));

    /*get erase table from FPGA RAM*/
    sema = &sdev->channel_ctl[channel].sema;    

    down(sema);
    if (copy_from_user(map, mecparam->erase_buffer, count * sizeof(struct erase_entry_map))) {
        ssd_log(SSD_LOG_ERR, "ssd_id_write: copy from user failed (line %d)\n", __LINE__);
        ret = -SSD_ERR_COPY_TO_USER;
        goto out;
    }
    eem = (struct erase_entry_map *)map;

    stparam.usr_buf = NULL;
    stparam.channel = channel;
    stparam.type = TABLE_ERASE_TYPE;

    ret = ram_table_read(sdev, &stparam);
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "fail to read erase ram table, channel = %u type = %u ,ret[%d] (line %d)\n", 
            stparam.channel, stparam.type, ret, __LINE__);
        goto out;
    }

    erase_table = (struct erase_entry *)sdev->kbuftable_cpu[channel];

    for (i = 0; i < count; i++) {
        phy_block = (eem + i)->block;

        (erase_table + phy_block)->erase_times = (eem + i)->ee.erase_times;
        (erase_table + phy_block)->reserved = (eem + i)->ee.reserved;
        (erase_table + phy_block)->use_flag = (eem + i)->ee.use_flag;
        (erase_table + phy_block)->bad_flag = (eem + i)->ee.bad_flag;
    }

#if 0
    /*debug*/
    for(i = 0; i < count; i++) {
        ssd_log(SSD_LOG_ERR, "channel: %u, block: %u, erase: %u, bad: %u\n", 
            channel, i, (erase_table + i)->erase_times, (erase_table + i)->bad_flag);
    }
#endif

    /*copy to FPGA RAM*/   
    ret = ssd_write_tables_to_ram(sdev, channel, SSD_FLAG_WRITE_ERASE_TABLE);
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "init failed in writing erase table to ram, error: %d, channel[%u] (line %d)\n", ret, channel, __LINE__); 
        goto out;
    }

    ssd_log(SSD_LOG_EMG, "update erase table successfully, channel: %u, (line %d)\n", channel, __LINE__);
	
    /*flush table*/
//	ret = ssd_sys_flush(sdev, FLUSH_NORMAL_TYPE);

out:
    up(sema);
    if(map)
        kfree(map);
fail:
    return ret;
}

static inline void ssd_mark_erase_table(struct ssd_hash_node *node, struct addr_entry *addr, struct erase_entry *erase)
{
    ssd_u16   len = node->len ? BT_DATA_BLOCK_NUM : BT_INDEX_BLOCK_NUM;
    ssd_u16   start, logic_block, phy_block;

    start = (ssd_u16)node->block; 
    
    for (logic_block = start; logic_block < start + len; logic_block++) {
        phy_block = (addr + logic_block)->pb_addr;
        // mark used
        (erase + phy_block)->use_flag = 1;
    }
}

static int ssd_rebuild_erasetable(struct ssd_device *sdev, char *buffer)
{
    int       normal_uninstall;
    int       channel;
    struct erase_entry   *erase_id;
    struct addr_entry    *addr;
    struct erase_entry   *erase;

    int hash_list;
    struct ssd_hash_node *node;
    struct ssd_hash_head *head;
    struct channel_hash_list *ch_hash;

    normal_uninstall = sdev->meta.metadata_magic == FLUSH_MAGIC_CLOSE;
    addr = (struct addr_entry *)buffer;
    erase = (struct erase_entry *)(buffer + SSD_TABLE_SIZE);

    if (normal_uninstall)
        return SSD_OK;

    for (channel = 0; channel < SSD_MAX_CHANNEL; channel++) {

        //mark all erase tables unused expect reserved blocks
        for (erase_id = erase + RESERVE_BLOCKS_PER_CHANNEL / 2;
             erase_id < erase + SSD_MAX_PHYBLOCKS_PER_CHANNEL / 2; erase_id++) {
            erase_id->use_flag = 0;
        }
        for (erase_id = erase + SSD_MAX_PHYBLOCKS_PER_CHANNEL / 2 + RESERVE_BLOCKS_PER_CHANNEL / 2; 
             erase_id < erase + SSD_MAX_PHYBLOCKS_PER_CHANNEL; erase_id++) {
            erase_id->use_flag = 0;
        }

        //mark all used block according to id_hash and addr_table
        ch_hash = &(sdev->ch_hash[channel]);
        for (hash_list = 0; hash_list < SSD_HASH_SIZE; hash_list++) {
            head = &(ch_hash->head[hash_list]);
            node = head->first;
            while (node) {
                prefetch(node->next);
                // mark block used (need not to be erased)
                ssd_mark_erase_table(node, addr, erase);
                node = node->next;
            }
        }

        erase += SSD_TABLE_SIZE * SSD_TABLE_TYPE_NUM / sizeof(struct erase_entry);
        addr += SSD_TABLE_SIZE * SSD_TABLE_TYPE_NUM / sizeof(struct addr_entry);
    }

    return SSD_OK;
}

static inline int ssd_write_tables_to_ram(struct ssd_device *sdev, ssd_u8 channel, ssd_u8 flag)
{
    ssd_u64  cmd;
    int      ret;

    cmd = cmd_value(0, ADDR_TABLE_PAGES, flag);
    ret = channel_op(sdev, channel, cmd);
    return ret;
}

static inline int ssd_erase_phyblock(struct ssd_device *sdev, ssd_u8 channel, ssd_u16 block)
{
    ssd_u64   cmd;
    int       ret;

    cmd = cmd_value(block * SSD_PAGES_PER_BLOCK, 1, SSD_FLAG_ERASETABLES);
    ret = channel_op(sdev, channel, cmd);
    return ret;
}

static int ssd_erase_unused_block(struct ssd_device *sdev, ssd_u8 channel, struct erase_entry *buf)
{
    int    ret;
    struct erase_entry *erase_id;

    //erase all unused block in first two planes 
    for (erase_id = buf + RESERVE_BLOCKS_PER_CHANNEL / 2; erase_id < buf + SSD_MAX_PHYBLOCKS_PER_CHANNEL / 2; erase_id++) {
        if (erase_id->use_flag || erase_id->bad_flag)
                continue;


        // erase blocks of unused and not bad
        ret = ssd_erase_phyblock(sdev, channel, erase_id - buf);
        if (unlikely(ret != SSD_OK))
            return ret;
    }

    //erase all unused block in last two planes 
    for (erase_id += RESERVE_BLOCKS_PER_CHANNEL / 2; erase_id < buf + SSD_MAX_PHYBLOCKS_PER_CHANNEL; erase_id++) {
        if (erase_id->use_flag || erase_id->bad_flag)
                continue;


        // erase blocks of unused and not bad
        ret = ssd_erase_phyblock(sdev, channel, erase_id - buf);
        if (unlikely(ret != SSD_OK))
            return ret;
    }

    return SSD_OK;
}

static int ssd_updata_tables(struct ssd_device *sdev, char *buffer, int normal_uninstall)
{
    ssd_u8       channel;
    int       ret;
    struct addr_entry    *addr;
    struct erase_entry   *erase;

    addr = (struct addr_entry *)buffer;
    erase = (struct erase_entry *)(buffer + SSD_TABLE_SIZE);

    for (channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
        // write erase table
        memcpy(sdev->kbuftable_cpu[channel], erase, SSD_TABLE_SIZE);
        ret = ssd_write_tables_to_ram(sdev, channel, SSD_FLAG_WRITE_ERASE_TABLE);
        if (ret != SSD_OK) {
            ssd_log(SSD_LOG_ERR, "init failed in writing erase table to ram, error: %d, channel[%u] (line %d)\n", ret, channel, __LINE__); 
            return ret;
        }


        // write addr table
        memcpy(sdev->kbuftable_cpu[channel], addr, SSD_TABLE_SIZE);
        ret = ssd_write_tables_to_ram(sdev, channel, SSD_FLAG_WRITE_ADDR_TABLE);
        if (ret != SSD_OK) {
            ssd_log(SSD_LOG_ERR, "init failed in writing addr table to ram, error :%d, channel[%u] (line %d)\n", ret, channel, __LINE__);
            return ret;
        }

        // erase all the unused block
        if (!normal_uninstall) {
            ret = ssd_erase_unused_block(sdev, channel, erase);
            if (ret != SSD_OK) {
                ssd_log(SSD_LOG_ERR, "init failed in erasing unused block, error :%d, channel[%u] (line %d)\n", ret, channel, __LINE__);
                return ret;
            }
        }

        erase += SSD_TABLE_SIZE;
        addr += SSD_TABLE_SIZE;
    }

    return SSD_OK;
}

static int ssd_sys_init(struct ssd_device *sdev)
{
    int     ret;
    int     normal_uninstall;
    char    *buffer;

    // step1 : search and read metadata
    ret = ssd_search_metadata(sdev);

    // can not find valid metadata
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "SSD: init failed in search metadata, error :%d! (line %d)\n", ret, __LINE__);
        return ret;
    }

    buffer = sdev->kbuf_flush;
    memset(buffer, 0, sdev->meta.block_num * SSD_BLOCK_SIZE);
    
    // step2 : read tabledata according to metadata
    ret = ssd_read_flush_tabledata(sdev, buffer);
    if (unlikely(ret != SSD_OK)) {
        ssd_log(SSD_LOG_ERR, "init failed in read flush tabledata, error :%d! (line %d)\n", ret, __LINE__);
        goto OUT;
    }

    // step3 : rebuild hash list
    ret = ssd_rebuild_hash(sdev, buffer);
    if (unlikely(ret != SSD_OK)) {
        ssd_log(SSD_LOG_ERR, "init failed in rebuild hash, error :%d! (line %d)\n", ret, __LINE__);
        goto OUT;
    }

    // step4 : rebuild erase_table 
    // return value is always SSD_OK
    ssd_rebuild_erasetable(sdev, buffer);

    // step5 : write addr table & erase table to ram
    normal_uninstall = sdev->meta.metadata_magic == FLUSH_MAGIC_CLOSE;
    ret = ssd_updata_tables(sdev, buffer, normal_uninstall);

OUT:
    return ret;
}

static int ssd_sys_open_newcard(struct ssd_device *sdev)
{
    int ret, channel;

    if (g_opened_flag == SSD_OPENED)
        return -SSD_ERR_NOT_CLOSED;
        
    g_ssd_log_fp = ssd_log_open(LOG_PATH, O_CREAT | O_RDWR | O_APPEND, 0);
    if(!g_ssd_log_fp)
        printk(KERN_INFO "ssd_log_open failed\n");
    
    ret = ssd_create_slab(sdev);
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "ssd_open_newcard: ssd_create_slab failed (line %d)\n", __LINE__);
        goto err;
    }

    ssd_init_hash(sdev);
    
    /* init hash table head for each channel */ 
    ret = ssd_sys_init_newcard(sdev);
    if(SSD_OK != ret) {
        ssd_log(SSD_LOG_ERR, "ssd_open_newcard: ssd_sys_init_newcard failed, ret[%d] (line %d)\n", ret, __LINE__);
        ssd_destory_slab(sdev);
        goto err;
    }
        
    // ssd_load_hash_table(sdev);
    ret = ssd_reload_bitmap(sdev);
    if(SSD_OK != ret) {
        ssd_log(SSD_LOG_ERR, "ssd_open_newcard: ssd_reload_bitmap failed, ret[%d] (line %d)\n", ret, __LINE__);
        ssd_destory_slab(sdev);
        goto err;
    }
    
    /*set for free space*/
    for(channel = 0; channel < SSD_MAX_CHANNEL; channel++)
        free_blocks[channel] = sdev->free_blocks[channel];

	/*init flush semaphore*/
	init_MUTEX(&flush_sema);

    g_opened_flag = SSD_OPENED;

    return SSD_OK;

err:
    ssd_log_close();
    return SSD_ERROR;
}

static int ssd_sys_init_newcard(struct ssd_device *sdev)
{
    int ret;
    ssd_u8 channel;
    
    /*creat new erase and addr table for one channel and erase all blocks*/
    for (channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
        ret =ssd_sys_init_newcard_in_channel(sdev, channel);
        if (ret != SSD_OK)
            return ret;
    }
    
    
    /* create meta data */
    ssd_init_meta(sdev);
    
    return SSD_OK;
}

static int ssd_sys_init_newcard_in_channel(struct ssd_device *sdev, ssd_u8 channel)
{
    int ret;
    int block;
    channel_op_control_t *ch_ctl = &(sdev->channel_ctl[channel]);
    struct semaphore *sema = &(ch_ctl->sema);
    
    down(sema);
    
    /* create a new erase count table and write it to fpga ram */
    ret = ssd_erase_table_init(sdev, channel);
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "Newcard: create new erase table failed in channel[%u], ret = %d (line %d)\n", channel, ret, __LINE__);
        goto out;
    }
    
    /* create a new addr map table and write it to fpga ram */
    ret = ssd_addr_table_init(sdev, channel);
    if (ret != SSD_OK) {
        ssd_log(SSD_LOG_ERR, "Newcard: create new addr table failed in channel[%u], ret = %d (line %d)\n", channel, ret, __LINE__);
        goto out;
    }
    
    /* erase all the blocks */
    printk("Start erasing all blocks in channel %u .... \n", channel);
    for (block = 0; block < SSD_MAX_PHYBLOCKS_PER_CHANNEL; block++) {
        ret = ssd_erase_phyblock(sdev, channel, block);
        if (ret != SSD_OK) {
            ssd_log(SSD_LOG_ERR, "Newcard: erase all blocks failed in channel[%u], block[%d] ret = %d (line %d)\n", channel, block, ret, __LINE__);
            goto out;
        }
    }


out:
    up(sema);

    return ret;
}

static int ssd_erase_table_init(struct ssd_device *sdev, ssd_u8 channel)
{
    /* create a new erase count table */
    create_erase_table(sdev, channel);
    return ssd_write_tables_to_ram(sdev, channel, SSD_FLAG_WRITE_ERASE_TABLE);
}

static void create_erase_table(struct ssd_device *sdev, ssd_u8 channel)
{
    ssd_u16 *table = (ssd_u16 *)(sdev->kbuftable_cpu[channel]);
    int i;
  
    /* 
     * create an addr map table and flush to fpga ram
     * each entry in table is:   0~12 erase count, 13reserve,14 used block flag, 15 bad flag
     * block 0x0~0x9 and 0x1000~0x1009 used for persistence information(store tables)
     *
     */
    for (i = 0; i < SSD_MAX_PHYBLOCKS_PER_CHANNEL; i++)
    {
    /*
        if (channel == 0 && (i == 86 || i == 87))
        {
            table[i] = 0x0000;
        }
        else */
            table[i] = 0x0001;//0x4000;
    }
}

static int ssd_addr_table_init(struct ssd_device *sdev, ssd_u8 channel)
{
    /* create a new addr map table */
    create_addrmap_table(sdev, channel);
    return ssd_write_tables_to_ram(sdev, channel, SSD_FLAG_WRITE_ADDR_TABLE);
}

static void create_addrmap_table(struct ssd_device *const sdev, ssd_u8 channel)
{
    ssd_u16 *table = (ssd_u16 *)(sdev->kbuftable_cpu[channel]);
    ssd_u16 phystart;
    int i;
    /* 
     * create an addr map table and flush to fpga ram
     * each entry in table is:   0~12 the phy block addr, 13~14 researverd, 15 entry valid flag
     */
    (void)memset(table, 0, SSD_PAGES_PER_TABLE * SSD_PAGE_SIZE);

    /* for plane0 and plane1 */
    phystart = PHY_PLANE01_BLOCK_VALID_START;
    for (i = 0; i < 2 * PLANE_VALID_BLOCKS; i++, phystart++, table++) {
        *table = (phystart & ADDR_MASK);
    }

    /* for plane2 and plane3 */
    phystart = PHY_PLANE23_BLOCK_VALID_START;
    for (i = 2 * PLANE_VALID_BLOCKS; i < 4 * PLANE_VALID_BLOCKS; i++, phystart++, table++) {
        *table = (phystart & ADDR_MASK);
    }

    table = (ssd_u16 *)(sdev->kbuftable_cpu[channel]);
}

static inline void ssd_init_meta(struct ssd_device *sdev)
{
    sdev->meta.metadata_magic = FLUSH_MAGIC_NORMAL;
    sdev->meta.block_num = 1;
    sdev->meta.last_block_used_bytes = 2 * 1024 * 1024;
    sdev->meta.id_num = 0;
    sdev->meta.version = 1;
    sdev->meta.timestamp = 0;
    sdev->meta.table_block_pos[0] = RESERVE_BLOCKS_META;
    sdev->meta.table_block_pos[1] = sdev->meta.table_block_pos[0] + 
                               SSD_MAX_CHANNEL_EACH_TABLE * RESERVE_BLOCKS_PER_CHANNEL;
    
    /* init at last page, so first flush will at page 0 of next block*/
    sdev->meta_pos.page = SSD_PAGES_PER_BLOCK - 1; 
    sdev->meta_pos.block_pos = 0;
    
    ssd_log(SSD_LOG_INFO, "init meta: block[0]=%u, block[1]= %u\n", sdev->meta.table_block_pos[0], sdev->meta.table_block_pos[1]);
}

static int ssd_block_ioctl(struct block_device *bdev, fmode_t mode, unsigned int cmd, unsigned long arg)
{
    int ret;
    struct ssd_blockid id;
    struct ssd_read_param rparam;
    struct ssd_write_param wparam;
//    struct ssd_write_retry_param wrparam;
    struct ssd_table_param tprarm;
    struct ssd_readid_param ridparam; 
    struct ssd_regwr_param regwrparam;
    struct ssd_regrd_param regrdparam;
    struct ssd_mark_bad_param mbparam;
    struct ssd_mark_erase_count_param mecparam;
    void __user *argp = (void __user *)arg;
    struct ssd_device *sdev = bdev->bd_disk->private_data;
    ssd_u64 ret_value;
    struct ssd_meta_version version;

    switch (cmd) {
    case IOCTL_OPEN: 
        ret = ssd_sys_open(sdev);
        break;
    case IOCTL_READ_FLUSH:
        if (copy_from_user(&version, argp, sizeof(struct ssd_meta_version))) {
            ret = -SSD_ERR_COPY_FROM_USER;
            break;
        }
        ret = ssd_read_flush(sdev, &version);
        break;
    case IOCTL_CLOSE: 
        ret = ssd_sys_close(sdev);
        break;
    case IOCTL_FLUSH:
        ret = ssd_sys_flush(sdev, FLUSH_NORMAL_TYPE);
        break;    
    case IOCTL_DELETE:
        if (copy_from_user(&id, argp, sizeof(struct ssd_blockid))) {
            ret = -SSD_ERR_COPY_FROM_USER;
            break;
        }
        ret = ssd_id_delete(sdev, &id);
        break;
    case IOCTL_WRITE:
        if (copy_from_user(&wparam, argp, sizeof(struct ssd_write_param))) {
            ret = -SSD_ERR_COPY_FROM_USER;
            break;
        }
        ret = ssd_id_write(sdev, &wparam);
        break;
    case IOCTL_READ:
        if (copy_from_user(&rparam, argp, sizeof(struct ssd_read_param))) {
            ret = -SSD_ERR_COPY_FROM_USER;
            break;
        }
        ret = ssd_id_read(sdev, &rparam);
        break;
    case IOCTL_READ_TABLE:
        if (copy_from_user(&tprarm, argp, sizeof(struct ssd_table_param))) {
            ret = -SSD_ERR_COPY_FROM_USER;
            break;
        }
        ret = ssd_table_read(sdev, &tprarm);
        break;
    case IOCTL_WRITE_RETRY:
        /*
        if(copy_from_user(&wrparam, argp, sizeof(struct ssd_write_retry_param))) {
            ret = -SSD_ERR_COPY_FROM_USER;
            break;
        }
        ret = ssd_id_write_retry(sdev, &wrparam);
        */
        ssd_log(SSD_LOG_ERR, "We do not support IOCTL_WRITE_RETRY after 1-1-0-4\n");
        ret = SSD_ERROR;
        break;
    case  IOCTL_READ_ID_TABLE:
        if (copy_from_user(&ridparam, argp, sizeof(struct ssd_readid_param))) {
            ret = -SSD_ERR_COPY_FROM_USER;
            break;
        }
        ret = -ssd_read_idtable(sdev, &ridparam);/*ret of the func is always positive*/
        break;        
    case IOCTL_SYS_INIT:  
        ret = ssd_sys_open_newcard(sdev);
        break;
    case IOCTL_UNITTEST:
        //ssd_unittest();
        ret = SSD_OK;
        break;
    case IOCTL_REG_READ:
        if (copy_from_user(&regrdparam, argp, sizeof(struct ssd_regrd_param))) {
            ret = -SSD_ERR_COPY_FROM_USER;
            break;
        }
        ret_value = ssd_reg64_read(sdev->conf_regs + regrdparam.addr);
        ret = copy_to_user(regrdparam.ret_value, &ret_value, sizeof(ssd_u64));
        //printk("value is %llx\n", ret_value);
        break;
    case IOCTL_REG_WRITE:
        //printk("reg write\n");
        if (copy_from_user(&regwrparam, argp, sizeof(struct ssd_regwr_param))) 
        {
            ret = -SSD_ERR_COPY_FROM_USER;
            break;
        }
        //printk("addr is 0x%llx, value is 0x%llx\n", regwrparam.addr, regwrparam.value);

        ssd_reg64_write(sdev->conf_regs + regwrparam.addr, regwrparam.value);
        ret = SSD_OK;
        break;
    case IOCTL_MARK_BAD_BLOCK:
        if (copy_from_user(&mbparam, argp, sizeof(struct ssd_mark_bad_param))) {
            ret = -SSD_ERR_COPY_FROM_USER;
            break;
        }

        ret = ssd_mark_bad_block(sdev, &mbparam);
        break;
    case IOCTL_MARK_ERASE_COUNT:
        if (copy_from_user(&mecparam, argp, sizeof(struct ssd_mark_erase_count_param))) {
            ret = -SSD_ERR_COPY_FROM_USER;
            break;
        }

        ret = ssd_mark_erase_count(sdev, &mecparam);
        break;
    default:
        ret = SSD_ERROR;
        break;
   }

    if (ret < 0)
        ret = -ret;
    return ret;
}

static int create_ssd_proc(void)
{

    proc_ssd_dir = proc_mkdir("Baidu_ssd_info", NULL); 
    if (proc_ssd_dir == NULL) {
        printk(KERN_ERR "SSD: create proc dir /proc/Baidu_ssd_info failed!\n");
        return SSD_ERROR;
    }
    
    proc_ssd_speed_entry = create_proc_read_entry("speed", 0755, proc_ssd_dir, proc_speed_read, NULL);
    if (proc_ssd_speed_entry == NULL) {
        printk(KERN_ERR "SSD: create proc dir /proc/Baidu_ssd_info/speed failed!\n");
        return SSD_ERROR;
    }

    //TODO:    
    return SSD_OK;
}
static int del_ssd_proc(void)
{
    if (proc_ssd_speed_entry)
        remove_proc_entry("speed", proc_ssd_dir);
    if (proc_ssd_dir)
        remove_proc_entry("Baidu_ssd_info", NULL);
    
    return SSD_OK;
}    

static int proc_speed_read(char *page, char **start, off_t off, int count, int *eof, void *data)
{
    int i;
    int len = 0;
    char *p = page;
    unsigned int total_read = 0;
    unsigned int total_write = 0;
    unsigned int total_delete = 0;
    int total_free = 0;

    //snapshot
    if (off == 0)
        for (i = 0; i < SSD_MAX_CHANNEL; i++){
            read_count_buf[i] = read_count[i];
            write_count_buf[i] = write_count[i];
            delete_count_buf[i] = delete_count[i];
            free_blocks_buf[i] = free_blocks[i];
        }

    //calculate total count
    for (i = 0; i < SSD_MAX_CHANNEL; i++){
        total_read += read_count_buf[i];
        total_write += write_count_buf[i];
        total_delete += delete_count_buf[i];
        total_free += free_blocks_buf[i];
    }

    //create proc data
    len = snprintf(p, 85, "Total: Rd %u Wr %u Del %u Free %d\n",
           total_read, total_write, total_delete, total_free);
    if (len < 0)
        return 0;
    p += len;

    for (i = 0; i < SSD_MAX_CHANNEL; i++)
    {
        len = snprintf(p, 85, "Channel[%u]: Rd %u Wr %u Del %u Free %d\n",
               i, read_count_buf[i], write_count_buf[i], delete_count_buf[i], free_blocks_buf[i]);
        if (len < 0)
             return 0;
        p += len;
    }

    len = p - page;
    
    if (len < 0)
        len = 0;
    
    return len;
}

static struct pci_device_id ssd_pci_tbl[] = {
    {VERDOR_ID_BAIDU, SSD_DEEPOCEAN_DEVICE_ID, PCI_ANY_ID, PCI_ANY_ID, },
    { 0,}
};


MODULE_DEVICE_TABLE(pci, ssd_pci_tbl);

static struct pci_driver ssd_driver = {
        .name            = SSD_DRIVER_NAME,
        .id_table        = ssd_pci_tbl,
        .probe           = ssd_probe,
        .remove          = ssd_remove,
};

struct block_device_operations ssd_ops = {
    .owner = THIS_MODULE,
    .open = ssd_block_open,
    .release = ssd_block_release,
    .ioctl = ssd_block_ioctl,
};

static void ssd_block_request(struct request_queue *q)
{
    struct request *req;

    while((req = blk_fetch_request(q)))
       __blk_end_request_all(req, 0);
}

static int ssd_add_device(struct ssd_device *sdev)
{
    int i;
    struct gendisk **gd = sdev->gdisk;
    struct request_queue *queue;

    memset(gd, 0, sizeof(sdev->gdisk));
    for (i = 0; i < SSD_MAX_CHANNEL; i++) {
        gd[i] = alloc_disk(0); /* no partition */
        if (!gd[i]) {
            printk(KERN_ERR "%s: alloc_disk failure\n", sdev->name);
            goto out;
        }
    }

    spin_lock_init(&sdev->queue_lock);
    queue = blk_init_queue(ssd_block_request, &sdev->queue_lock);
    if (queue == NULL)
        return -SSD_ERR_NOMEM;

    /* init gendisk one channel one device */
    for (i = 0; i < SSD_MAX_CHANNEL; i++) {
        gd[i]->major = ssd_major;
        gd[i]->first_minor = i;
        gd[i]->minors = SSD_MINORS;        /* no partition */
        gd[i]->fops = &ssd_ops;
        gd[i]->private_data = sdev; 
        gd[i]->queue = queue; 
        snprintf(gd[i]->disk_name, sizeof(gd[i]->disk_name), "%s%i", SSD_DEVICE_NAME, i);
        set_capacity(gd[i], 0);
        add_disk(gd[i]);
    }

    return SSD_OK;

out:
    while (i--) {
        put_disk(gd[i]);
        gd[i] = NULL;
    }
    return -SSD_ERR_NOMEM;

}
static int __init ssd_init_module(void)
{
    int ret;

    ssd_major = register_blkdev(ssd_major, SSD_DEVICE_NAME);
    if (ssd_major < 0) {
        printk(KERN_ERR "SSD: unable to get major number!\n");
        goto err_out;
    }

    /* ssd_dev is set on pci_register_deriver */
    ret = pci_register_driver(&ssd_driver);
    if ((ret != 0) || (ssd_dev == NULL)) { 
        printk(KERN_ERR "SSD: pci init error!\n");
        goto err_register_pci;
    }

    ret = ssd_add_device(ssd_dev);
    if (ret != SSD_OK)
        goto err_add_device;
    
    ret = create_ssd_proc();
    if (ret != SSD_OK) {
        printk(KERN_ERR "SSD: create proc dir failed\n");
        goto err_create_proc;
    }
    
    printk(KERN_INFO "SSD[%s]: load driver success, device name is %s, major is %d\n", SSD_MODULE_VERSION, ssd_dev->name, ssd_major);
    return 0;

err_create_proc:    
err_add_device: 
    pci_unregister_driver(&ssd_driver);
err_register_pci:
    unregister_blkdev(ssd_major, SSD_DEVICE_NAME);
    ssd_dev = NULL;
err_out:
    return -1;
}

static void ssd_del_device(struct ssd_device * sdev)
{
    ssd_u8 i; 
    ssd_u8 channels = SSD_MAX_CHANNEL;
    int ret;
    
    for (i = 0; i < channels; i++) {
        if (sdev->gdisk[i]) {
            del_gendisk(sdev->gdisk[i]);
            put_disk(sdev->gdisk[i]);
        }
    }

    if (sdev->queue) {
        blk_cleanup_queue(sdev->queue);
    }
    
    ret = del_ssd_proc();
    if (ret != SSD_OK)
        printk(KERN_WARNING "SSD: delete proc dir failed\n");
}

static void __exit ssd_remove_module(void)
{
    if ((g_opened_flag == SSD_OPENED) && ssd_dev)
    {
        printk("SSD[%s]: remove module and close the ssd!\n", SSD_MODULE_VERSION);
        (void)ssd_sys_close(ssd_dev);
    }
    if (ssd_dev)
        ssd_del_device(ssd_dev);

    pci_unregister_driver(&ssd_driver);

    unregister_blkdev(ssd_major, SSD_DEVICE_NAME);

    //ssd_log(SSD_LOG_INFO, "SSD: unload SSD Module, device name is %s\n", SSD_DEVICE_NAME);
}

 
module_init(ssd_init_module);
module_exit(ssd_remove_module);
MODULE_VERSION(SSD_MODULE_VERSION);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SSD DEV Team");
MODULE_DESCRIPTION("SSD driver");
/*----------------------------------------------------------------------------*
REV #  DATE       BY    REVISION DESCRIPTION
-----  --------  -----  ------------------------------------------------------
0001   11/10/24   wangyong   Initial version.
0002   13/07/09   suijulei   Version:1-2-0-0
*----------------------------------------------------------------------------*/

