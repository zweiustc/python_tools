#include "ssd_api.h"

#define MAX_FILE_SIZE 1024
#define BUFFER_LINE 128
#define BLOCK_PER_ROW 8

#define FPGAS_HUAWEI 4
#define VFLASH_PER_PFLASH_HUAWEI 4
#define PFLASH_PER_FPGA_HUAWEI 11
#define VFLASHES_PER_FPGA_HUAWEI (VFLASH_PER_PFLASH_HUAWEI * PFLASH_PER_FPGA_HUAWEI)

#define BLOCK_BASE_FLASH_BAIDU 4096
#define BAD_BLOCK_ERASE_COUNT 8191

#define PLANE_PER_CHANNEL 4

//#define debug

ssd_u16 huawei_block_to_baidu_block(int plane, ssd_u16 block_huawei)
{
	/*
	switch(plane) {
		case 0:
			block_baidu = block_huawei * 2;
			break;
		case 1:
			block_baidu = block_huawei * 2 + 1;
			break;
		case 2:
			block_baidu = BLOCK_BASE_FLASH_BAIDU + block_huawei * 2;
			break;
		case 3:
			block_baidu = BLOCK_BASE_FLASH_BAIDU + block_huawei * 2 + 1;

	}
	*/
	return BLOCK_BASE_FLASH_BAIDU * (plane / 2) + block_huawei * 2 + plane % 2;
}

int baidu_plane_to_baidu_channel(int plane)
{
	return plane / PLANE_PER_CHANNEL;
}

int plane_offset(int plane)
{
	return plane % PLANE_PER_CHANNEL;
}

int huawei_vflash_to_baidu_plane(int vflash)
{
	return vflash * VFLASH_PER_PFLASH_HUAWEI % VFLASHES_PER_FPGA_HUAWEI + vflash / PFLASH_PER_FPGA_HUAWEI;
}

int huawei_vflash_to_baidu_channel(int vflash)
{
	return baidu_plane_to_baidu_channel(huawei_vflash_to_baidu_plane(vflash));
}

int huawei_fpga_vflash_to_baidu_channel(int fpga, int vflash)
{
	return fpga * PFLASH_PER_FPGA_HUAWEI + huawei_vflash_to_baidu_channel(vflash);
}

void huawei2baidu(int fpga, int vflash, int start_block, int erase_count[], ssd_u8 **baidu)
{
	ssd_u8 channel;
	int plane;
	ssd_u16 block_huawei, block_baidu, index;
	struct erase_entry_map *eem_channel, *eem;

	/*   huawei(4 FPGAS)                        baidu(44 channels)
	 *|FPGA|FPGA|FPGA|FPGA|                |channel|channel|channel|...
	 *         / -11- \                           /   -2-   \
	 *|PFLASH|PFLASH|PFLASH|...                  |flash|flash|
	 *      /  -4-   \                                /  -2-  \
	 *|VFLASH|VFLASH|VFLASH|VFLASH|                  |plane|plane|
	 */
	channel = huawei_fpga_vflash_to_baidu_channel(fpga, vflash);

	eem_channel = (struct erase_entry_map *)baidu[channel];
	for(index = 0, block_huawei = start_block; (block_huawei < start_block + BLOCK_PER_ROW) && (index < BLOCK_PER_ROW); block_huawei++, index++) {
		plane = plane_offset(huawei_vflash_to_baidu_plane(vflash));
		block_baidu = huawei_block_to_baidu_block(plane, block_huawei);
#if 0
		printf("fpga: %d, flash: %d, huawei: %d, channel: %u, baidu: %d, erase: %d\n", 
				fpga, vflash, block_huawei, channel, block_baidu, erase_count[index]);
#endif
		eem = eem_channel + block_baidu;
		eem->block = block_baidu;
		if(erase_count[index] == -1) {
			eem->ee.erase_times = BAD_BLOCK_ERASE_COUNT;
			eem->ee.bad_flag = 1;
		} else {
			eem->ee.erase_times = erase_count[index];
			eem->ee.bad_flag = 0;
		}
		eem->ee.reserved = 0;
		eem->ee.use_flag = 0;
	}
	
}


void help_info(char *cmd)
{
	/*
	printf("%s -f <erase table file path> -g <FPGA in Huawei>[0~3, default: all] -s <flash in Huawei>[0~43, default: all]\n", cmd);
	*/
	printf("%s -f <erase table file path>\n", cmd);
}

int main(int argc, char *argv[])
{
	int ret = 0, c;
	ssd_u8 channel;
	ssd_u8 *baidu_table[SSD_MAX_CHANNEL] = {NULL};
	char file_path[MAX_FILE_SIZE] = {'\0'};
	char *file = NULL;
	int fpga = -1, flash = -1, start_block;
	int fpga_no, flash_no;
	int erase_count[BLOCK_PER_ROW];
	FILE *fp = NULL;
	char buffer[BUFFER_LINE];

	if(argc < 2) {
		help_info(argv[0]);
		return -1;
	}

	while((c = getopt(argc, argv, "f:g:s:h")) > 0) {
		switch(c) {
			case 'h':
				help_info(argv[0]);
				return -1;
			case 'f':
				strncpy(file_path, optarg, MAX_FILE_SIZE);
				file = file_path;
				break;
			case 'g':
				fpga = atoi(optarg);
				if(fpga < 0 || fpga > 3) {
					help_info(argv[0]);
					return -1;
				}
				break;
			case 's':
				flash = atoi(optarg);
				if(flash < 0 || flash > 43) {
					help_info(argv[0]);
					return -1;
				}
				break;
			default:
				help_info(argv[0]);
				return -1;
		}
	}

	if(!file) {
		printf("file_path?\n");
		help_info(argv[0]);
		return -1;
	}

	for(channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
		baidu_table[channel] = malloc(SSD_MAX_PHYBLOCKS_PER_CHANNEL * sizeof(struct erase_entry_map));
		if(!baidu_table[channel])
			break;
	}

	if(channel != SSD_MAX_CHANNEL) {
		printf("fail to alloc mem for baidu table\n");
		goto fail;
	}

	fp = fopen(file, "r");
	if(!fp) {
		printf("fail to open file[%s]\n", file);
		goto fail;
	}
	
	while(fgets(buffer, BUFFER_LINE, fp)) {
//		printf("get buffer: %s", buffer);
		sscanf(buffer, "%d %d %d:\t%d %d %d %d %d %d %d %d", 
				&fpga_no, &flash_no, &start_block, \
				&erase_count[0], &erase_count[1], &erase_count[2], &erase_count[3], \
				&erase_count[4], &erase_count[5], &erase_count[6], &erase_count[7]);

		huawei2baidu(fpga_no, flash_no, start_block, erase_count, baidu_table);
	}

	/*debug*/
#ifdef debug 
	struct erase_entry_map *eem;
	ssd_u16 block_index;
	for(channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
		eem = (struct erase_entry_map *)baidu_table[channel];
		for(block_index = 0; block_index < SSD_MAX_PHYBLOCKS_PER_CHANNEL; block_index++) {
			printf("channel: %u, block: %u, erase: %u, bad: %d\n",
					channel, (eem + block_index)->block, (eem + block_index)->ee.erase_times, (eem + block_index)->ee.bad_flag);
		}
		
	}
#endif

	ret = SSD_open(0);
	if(ret != SSD_OK) {
		printf("fail to open SSD %d\n", ret);
		goto out;
	}
#if 0
	ret = SSD_mark_erase_count(0, (struct erase_entry_map *)baidu_table[1], SSD_MAX_PHYBLOCKS_PER_CHANNEL, 1);
	if(ret != SSD_OK)
		printf("mark erase count error, channel\n");
#else
	for(channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
		ret = SSD_mark_erase_count(0, (struct erase_entry_map *)baidu_table[channel], SSD_MAX_PHYBLOCKS_PER_CHANNEL, channel);
		if(ret != SSD_OK)
			printf("mark erase count error, channel: %u\n", channel);
	}
#endif

	SSD_close(0);

out:
	if(fp)
		fclose(fp);
	
fail:
	for(channel = 0; channel < SSD_MAX_CHANNEL; channel++)
		if(baidu_table[channel])
			free(baidu_table[channel]);

	printf("return value: %d\n", ret);
	return ret;
}
