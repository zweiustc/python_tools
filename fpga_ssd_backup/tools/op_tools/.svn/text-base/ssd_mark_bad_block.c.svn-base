#include "ssd_api.h"

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

int main(int argc, char* argv[])
{
	int ret = 0, c, block = -1, channel = -1;
	struct BlockId id;

	while((c = getopt(argc, argv, "i:b:c:h")) > 0) {
		switch(c) {
			case 'h':
				printf("%s -i<ID> [-b<block>-c<channel>]\n", argv[0]);
				goto out;
			case 'i':
				id = char2id(optarg);
				ret = SSD_mark_bad_block(MONITOR_DEV, PARAM_ID, id, 0, 0);
				goto out;
			case 'b':
				block = atoi(optarg);
				if(block < 0 || block >= 8000) {
					printf("arg -b <%d> error, [0, 8000)\n", block);
					ret = -1;
					goto out;
				}
				break;
			case 'c':
				channel = atoi(optarg);
				if(channel < 0 || channel >= 44) {
					printf("arg -c <%d> error, [0, 44)\n", channel);
					ret = -1;
					goto out;
				}
				break;
			default:
				printf("%s -i<ID> [-b<block>-c<channel>]\n", argv[0]);
				goto out;
			
		}
	}
				
	if(-1 == block || -1 == channel) {
		printf("need -b <block> && -c <channel>\n");
		ret = -1;
		goto out;
	}

	ret = SSD_mark_bad_block(MONITOR_DEV, PARAM_BLOCK, id, (ssd_u16)block, (ssd_u16)channel);
out:
	printf("mark block ret: %d\n", ret);
	return ret;
}
