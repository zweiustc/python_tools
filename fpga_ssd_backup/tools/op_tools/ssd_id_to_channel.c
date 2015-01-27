#include "ssd_api.h"

#define SSD_S6_NUMS 4
#define SSD_CHANNELS_IN_S6 11

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

int id2channel(struct BlockId id)
{
	return (id.m_nLow % SSD_S6_NUMS) * SSD_CHANNELS_IN_S6 + (id.m_nLow / SSD_S6_NUMS) % SSD_CHANNELS_IN_S6;
}

int main(int argc, char* argv[])
{
	printf("%s in channel %d\n", argv[1], id2channel(char2id(argv[1])));	

	return 0;
}

