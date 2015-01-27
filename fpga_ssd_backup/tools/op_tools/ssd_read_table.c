#include <sys/types.h>
#include <unistd.h>
#include "ssd_api.h"

#define TABLESIZE (SSD_MAX_PHYBLOCKS_PER_CHANNEL * 2)

int read_erase_table(int channel, char *buf)
{
	return SSD_read_erasetable(channel, buf);
}

int read_addr_table(int channel, char *buf)
{
	return SSD_read_addrtable(channel, buf);
}

void print_table(char *table, int type, int detail)
{
	int i;
	struct addr_entry *addr;
	struct erase_entry *erase;
	int max_erase = 0, min_erase = 100000, average_erase = 0;
	int max_key = 0, min_key = 0;
	int bad[SSD_MAX_PHYBLOCKS_PER_CHANNEL] = {0};

	if(!type) {
		addr = (struct addr_entry *)table;
		if(detail)
			printf("logic_addr\tphy_addr\n");
		for(i = 0; i < SSD_MAX_PHYBLOCKS_PER_CHANNEL; i++) {
			if(detail)
				printf("%d\t%d\n", i, addr->pb_addr);
			addr++;
		}
	}
	else {
		erase = (struct erase_entry *)table;
		if(detail)
			printf("phy_addr\tbad_flag\tuse_flag\tera_count\n");

		for(i = 0; i < SSD_MAX_PHYBLOCKS_PER_CHANNEL; i++) {
			if(detail)
				printf("erase entry info: %d\t%d\t%d\t%d\n", i, erase->bad_flag, erase->use_flag, erase->erase_times);

			if(erase->bad_flag)
				bad[i] = 1;

			if(max_erase < erase->erase_times) {
				max_erase = erase->erase_times;
				max_key = i;
			}

			if(min_erase > erase->erase_times) {
				min_erase = erase->erase_times;
				min_key = i;
			}

			average_erase += erase->erase_times;
			erase++;
		}
		
		printf("---------------common info-----------------\n");
		for(i = 0; i < SSD_MAX_PHYBLOCKS_PER_CHANNEL; i++) {
			if(bad[i])
				printf("invalid physical block: %d\n", i);
		}

		average_erase /= SSD_MAX_PHYBLOCKS_PER_CHANNEL;
		printf("max erase times:[%d's] %d, min erase times:[%d's] %d, average erase times: %d\n",
				max_key, max_erase, min_key, min_erase, average_erase);
	}

}

int read_table(int channel, int type, int detail)
{
	char *buffer;
	int ret;

	buffer = (char *)malloc(TABLESIZE);
	if(!buffer) {
		printf("fail to alloc buffer\n");
		return -1;
	}
	
	switch(type) {
		case 0:
		case 2:
			goto addr_table;
		case 1:
			goto erase_table;
	}

addr_table:
	memset(buffer, 0, TABLESIZE);
	ret = read_addr_table(channel, buffer);
	if(0 != ret)	{
		printf("fail to read addr table\n");
		goto out;
	}

	printf("------------channel[%d]addr table------------\n", channel);
	print_table(buffer, 0, detail);
	if(0 == type)
		goto out;

erase_table:
	memset(buffer, 0, TABLESIZE);
	ret = read_erase_table(channel, buffer);
	if(0 != ret )	{
		printf("fail to read erase table\n");
		goto out;
	}

	printf("------------channel[%d]erase table------------\n", channel);
	print_table(buffer, 1, detail);

out:
	free(buffer);
	return ret;
}

int main(int argc, char *argv[])
{
	int ret, channel = -1, type = 2;
	int all = 0, detail = 0;
	int c;

	while((c = getopt(argc, argv, "c:t:adh")) > 0) {
		switch(c) {
			case 'h':
				printf("%s -c<channel> -t<table type: 0(addr table), 1(erase table), [defalt]2(addr + erase)>  \
						-a[all channels and tables] -d[detail]\n", argv[0]);
				return 0;

			case 'a':
				all = 1;
				break;
			
			case 'c':
				channel = atoi(optarg);
				if(channel < 0 || channel >= SSD_MAX_CHANNEL) {
					printf("arg error, channel[0, 44)\n");
					return -1;
				}
				break;

			case 't':
				type = atoi(optarg);
				if(type < 0 || type > 2) {
					printf("arg error, type[0, 2]\n");
					return -1;
				}
				break;

			case 'd':
				detail = 1;
				break;

			defalt:
				printf("args error\n");
				return -1;
		}
	}

	if(!all) {
		if(-1 == channel) {
			printf("error args, please input %s -h\n", argv[0]);
			return -1;
		}

		return read_table(channel, type, detail);
	}


	for(channel = 0; channel < SSD_MAX_CHANNEL; channel++) {
		ret = read_table(channel, type, detail);
		if(ret < 0)
			break;
	}

	return ret;
}
