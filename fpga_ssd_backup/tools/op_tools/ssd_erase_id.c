#include "ssd_api.h"

#define SSD_S6_NUMS 4
#define SSD_CHANNELS_IN_S6 11

int channel_goal = -1;

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
//	printf("%llu\n %llu\n", id.m_nHigh, id.m_nLow);

	return id;
}

int delete_all_file(const char *fid, const struct stat *st, int flag)
{
	int ret = -1;
	ret = SSD_delete(0, char2id(fid));
	printf("delete %s: %d\n", fid, ret);
	return ret;
}

int id2channel(struct BlockId id)
{
	return (id.m_nLow % SSD_S6_NUMS) * SSD_CHANNELS_IN_S6 + (id.m_nLow / SSD_S6_NUMS) % SSD_CHANNELS_IN_S6;
}

int delete_channel(const char *fid, const struct stat *st, int flag)
{
	int ret = 0;
	struct BlockId id_del;

	id_del = char2id(fid);
	if(id2channel(id_del) == channel_goal) {
		ret = SSD_delete(0, id_del);
		printf("delete id %s in channel %d\n", fid, channel_goal);
	}

	return ret;
}

int main(int argc, char* argv[])
{
	int ret = -1, c, channel;
	struct BlockId id_goal;
	
	ret = SSD_open(0);
	if(SSD_OK != ret) {
		printf("fail to open ssd %d\n", ret);
		goto out;
	}

	while((c = getopt(argc, argv, "c:i:ah")) > 0) {
		switch(c) {
			case 'h':
				printf("%s -i<ID> -c<channel> -a[all]\n", argv[0]);
				goto out;

			case 'a':
				/*erase all exist ID*/
				ret = SSD_ftw(0, delete_all_file, 0);
				break;
			case 'i':
				id_goal = char2id(optarg);
				printf("delete %s [%lx, %lx]\n", optarg, id_goal.m_nHigh, id_goal.m_nLow);
				ret = SSD_delete(0, id_goal);
				break;
			case 'c':
				channel_goal = atoi(optarg);
				if(channel_goal < 0 || channel_goal > 43) {
					printf("arg for channel is error [0, 43] %d\n", channel_goal);
					ret = channel_goal;
					goto out;
				}

				ret = SSD_ftw(0, delete_channel, 0);
				break;
		}
	}

out:
	SSD_close(0);

	return ret;
}

