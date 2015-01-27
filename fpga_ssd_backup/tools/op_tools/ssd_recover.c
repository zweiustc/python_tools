#include <pthread.h>
#include <stdlib.h>
#include "ssd_api.h"

#define   TABLESIZE (SSD_MAX_PHYBLOCKS_PER_CHANNEL * 2)
#define   erase_flag 1

struct BlockId ch_ID[44][2000];
int ch_ID_logic[44][2000];
int ch_IDnum[44] = {0};

int check_pass = 1;

/*for crc*/
unsigned long long table[256];
unsigned long long key;

#define CRC_DATA 0x1LL
#define CRC_POLY 0x11edc6f41LL
#define CRC_DATA_BIT (1024 * 8)
#define CRC_POLY_BIT 33
#define CRC_BLOCK_HEADER  32

/* 字符转换ID */
struct BlockId char2id(const char *str)
{
	struct BlockId id;
	int i;
	char ch;
	unsigned long long sum = 0;

	for(i = 0; i < 32; i++) {
		if(16 == i) {
			id.m_nHigh = sum;
			sum = 0;
		}
		ch = *(str + i);
		if(ch >= '0' && ch <= '9')
			sum = sum * 16 + ch - '0';
		else if(ch >= 'A' && ch <= 'F')
			sum = sum * 16 + ch - 'A' + 10;
		else
			sum = sum * 16 + ch - 'a' + 10;
	}

	id.m_nLow = sum;

	return id;
}

int SSD_recover_delete(int dev_num, int channel, struct BlockId id)
{
    struct ssd_blockid ssd_id;
    char ssd_path[SSD_MAX_DEVNAME];
    int local_ssd_fd = -1;
    int ret;
    
    snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", 0);
    local_ssd_fd = open(ssd_path, O_RDWR);
    if (local_ssd_fd < 0) {
        printf("SSD: /dev/ssd_Baidu%d open failed\n", 0);
        ret = SSD_ERR_OEPN_DEV_FAILED;
        goto out;
    }

    /* type conversion from c++ to c */
    ssd_id.m_nLow = id.m_nLow;
    ssd_id.m_nHigh = id.m_nHigh;
    ret = ioctl(local_ssd_fd, IOCTL_RECOVER_DELETE, &ssd_id);

    close(local_ssd_fd);

out:
    return ret;
}

/* 将构造的地址映射表或者擦除表更新到fpga的ram中 */

int SSD_update_tables_to_ram(int channel, int flag, char *buf)
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

    param.type = (ssd_u8)flag;
    param.channel = (ssd_u8)channel;
    param.usr_buf = buf;
    
    snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", 0);
    local_ssd_fd = open(ssd_path, O_RDWR);
    if (local_ssd_fd < 0) {
        printf("SSD: /dev/ssd_Baidu%d open failed\n", 0);
        ret = SSD_ERR_OEPN_DEV_FAILED;
        goto out;
    }
    
    ret = ioctl(local_ssd_fd, IOCTL_WRITE_TABLE, &param);
    
    close(local_ssd_fd);

out:
    return ret;
}

/*  调SSD_ftw时，将每个ID归到对应通道的ID数组中，并计数  */
int ID_to_channel(const char *fid, const struct stat *st, int flag)
{
    int channel = (int)st->st_rdev;
    int ch_id_num = ch_IDnum[channel];
    ch_ID[channel][ch_id_num] = char2id(fid);
    ch_ID_logic[channel][ch_id_num] = (int)st->st_ino;
    ch_IDnum[channel]++;
    return 0;
}

/* 擦除某个通道的一个物理块 */
int SSD_erase_phy_block(int channel, int phy_block)
{
    struct ssd_block_param param;
    char ssd_path[SSD_MAX_DEVNAME];
    int local_ssd_fd = -1;
    int ret;

    if(channel < 0 || channel > SSD_MAX_CHANNEL) {
        printf("error channel type!\n");
        ret = SSD_ERROR;
        goto out;
    }

    param.phy_block = (ssd_u16)phy_block;
    param.channel = (ssd_u8)channel;
    
    snprintf(ssd_path, SSD_MAX_DEVNAME, "%s%d", "/dev/ssd_Baidu", 0);
    local_ssd_fd = open(ssd_path, O_RDWR);
    if (local_ssd_fd < 0) {
        printf("SSD: /dev/ssd_Baidu%d open failed\n", 0);
        ret = SSD_ERR_OEPN_DEV_FAILED;
        goto out;
    }
    
    ret = ioctl(local_ssd_fd, IOCTL_ERASE_BLOCK, &param);
    
        if(ret != 0) {
            printf("recover: erase phyblock %d in channel %d, err = %d\n", phy_block, channel, ret);
            //return ret;
        }
    close(local_ssd_fd);

out:
    return ret;
}

int  erase_free_block(int channel, struct erase_entry *erase_table)
{
    int i = 0;
    int ret;

    for (i = 10; i < 8192; i++) {
        if(i >= 4096 & i <= 4105)
            continue;

        if((erase_table + i)->use_flag | (erase_table + i)->bad_flag) {
            //printf("channel %d: block %d not erase, bad_flag %u, use_flag %u\n", channel, i, (erase_table + i)->bad_flag, (erase_table + i)->use_flag);
            continue;
        }

        ret = SSD_erase_phy_block(channel, i);
        if(ret != 0) {
            //printf("recover: erase phyblock %d in channel %d, err = %d\n", phy_block, channel, ret);
            return ret;
        }
    }

    return SSD_OK;
}

void mark_correct_ID(int channel, int logic_block, struct addr_entry *addr_table, struct erase_entry *erase_table)
{
    int  i = 0;
    int  phy_block;

    for(i = 0; i < 5; i++) {
        phy_block = (addr_table + logic_block + i)->pb_addr;
        (erase_table + phy_block)->use_flag = 1;
    }
}

/* 检查从持久化信息中拿到的ID对应的数据是否正确 */
int check_ID_flush(char *ID_buf, int channel, struct addr_entry *addr_table, struct erase_entry *erase_table)
{
    int i, ret = -1;
    int id_num = ch_IDnum[channel];
    /*for crc*/
	int crc, nlen;
	int crc_calc;

    /*  check ID data right or not  */
    for(i = 0; i < id_num; i++) {
        ret = SSD_read(0, ch_ID[channel][i], ID_buf, BT_DATA_SIZE, 0);
        if(SSD_OK != ret) {
            printf("recover: channel %d read ID %016lx%016lx error %d\n", channel, ch_ID[channel][i].m_nHigh, ch_ID[channel][i].m_nLow, ret);
            if (ret == -9)
	     	    return -1;
            ret = SSD_recover_delete(0, channel, ch_ID[channel][i]);
            //printf("recover: channel %d delete ID %016lx%016lx error %d\n", channel, ch_ID[channel][i].m_nHigh, ch_ID[channel][i].m_nLow, ret);
            continue;
        }
        crc = *(int *)ID_buf;
		nlen = *(int *)(ID_buf + 4);

        /* 正常的ID长度不会超过10M，加此判断是为了避免长度错误导致计算crc出core */
        if(nlen > BT_DATA_SIZE || nlen <= CRC_BLOCK_HEADER) {
            printf("recover: ID %016lx%016lx nlen %d error\n", ch_ID[channel][i].m_nHigh, ch_ID[channel][i].m_nLow, nlen);
            //ret = SSD_recover_delete(0, channel, ch_ID[channel][i]);
            //printf("recover: channel %d delete ID %016lx%016lx error %d\n", channel, ch_ID[channel][i].m_nHigh, ch_ID[channel][i].m_nLow, ret);
            continue;
        }

        crc_calc = baidu_crc32_fast(ID_buf + CRC_BLOCK_HEADER, 0, nlen - CRC_BLOCK_HEADER, table);
        if(crc != crc_calc) {
            ret = SSD_recover_delete(0, channel, ch_ID[channel][i]);
            printf("recover: crc error! channel %d delete ID %016lx%016lx ret %d\n", channel, ch_ID[channel][i].m_nHigh, ch_ID[channel][i].m_nLow, ret);
            continue;
        }

        mark_correct_ID(channel, ch_ID_logic[channel][i], addr_table, erase_table);
    }

    return 0;
}

int  read_init_table(int channel, struct addr_entry *addr_table, struct erase_entry *erase_table)
{
    int ret, i = 0;
    
    /* 读擦除表、地址映射表 */
    ret = SSD_read_erasetable(channel, erase_table);
    if(0 != ret) {
        printf("recover: channel %d fail to read erase table[%d]\n", channel, ret);
		return -1;
    }

    ret = SSD_read_addrtable(channel, addr_table);
    if(0 != ret) {
        printf("recover: channel %d fail to read addr table[%d]\n", channel, ret);
		return -1;
    }

    /*  初始化擦除表 */
    for(i = 0; i < 8192; i++)
        (erase_table + i)->use_flag = 0;

}

void *work(void *args)
{
    int channel = (int)args;
    struct erase_entry *erase_table;
    struct addr_entry *addr_table;
    char *ID_buf;
    int ret;
    
    erase_table = (struct erase_entry *)malloc(TABLESIZE);
    addr_table = (struct addr_entry *)malloc(TABLESIZE);
    ID_buf = (char *)malloc(BT_DATA_SIZE);
	if(!erase_table || !addr_table || !ID_buf) {
		printf("recover: channel %d fail to alloc buffer\n", channel);
        check_pass = 0;
		goto out;
	}

    memset(erase_table, 0, TABLESIZE);
    memset(addr_table, 0, TABLESIZE);
    memset(ID_buf, 0, BT_DATA_SIZE);

    ret = read_init_table(channel, addr_table, erase_table);
    if(ret != 0) {
        printf("recover: read_init_table failed!\n");
        check_pass = 0;
        goto out;
    }

    ret = check_ID_flush(ID_buf, channel, addr_table, erase_table);
    if(ret != 0) {
        printf("ID check fail: error IDs\n");
        check_pass = 0;
        goto out;
    }

    /*ret = SSD_update_tables_to_ram(channel, TABLE_ADDR_TYPE, addr_table);
    if(ret != 0) {
        check_pass = 0;
        printf("update_addr_table fail, ret %d\n", ret);
        goto out;
    }*/

    ret = erase_free_block(channel, erase_table);
    if(ret != 0) {
        check_pass = 0;
        printf("erase_free_block fail, ret %d\n", ret);
    }

    ret = SSD_update_tables_to_ram(channel, TABLE_ERASE_TYPE, erase_table);
    if(ret != 0) {
        printf("update_erase_table fail, ret %d\n", ret);
        check_pass = 0;
        goto out;
    }

out:
    if(erase_table) free(erase_table);
    if(addr_table) free(addr_table);
    if(ID_buf) free(ID_buf);
}

int main(int argc, char *argv[])
{
    if(argc < 2) {
       printf("input start_channel, end_channel\n");
       return 1;
    }

    pthread_t id[44];
    int channel=0;
    int ch_start = atoi(argv[1]);
    int ch_end = atoi(argv[2]);

    if(ch_start < 0 || ch_end >= 44) {
        printf("channel error!");
        return 1;
    }

    /*  classify ID into different channel  */
    int ret = SSD_recover_open(0);
    if(SSD_OK != ret) {
        printf("recover: open error, ret = %d\n", ret);
        return 1;
    }

    /*  for crc */
    key = mod2(CRC_DATA, CRC_POLY, CRC_POLY_BIT, CRC_DATA_BIT);
	gen_table(table, key);

    ret = SSD_ftw(0, ID_to_channel, 0);
    if(SSD_OK != ret) {
        printf("ID_to_channel error: %d\n", ret);
        return 1;
    }

    printf("channel 0 : ID_num %d\n", ch_IDnum[0]);

    for(channel = ch_start; channel <= ch_end; channel++)
        pthread_create(&id[channel], NULL, work, (void *)channel);

    for(channel = ch_start; channel <= ch_end; channel++)
        pthread_join(id[channel], NULL);

    if(check_pass == 0) 
        return -1;

    printf("check pass!\n");

    //SSD_close(0);

    return 0;
}


