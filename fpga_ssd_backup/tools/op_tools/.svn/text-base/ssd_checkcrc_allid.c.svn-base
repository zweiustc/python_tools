#include "ssd_api.h"

#define SIZE (10 * 1024 * 1024)

struct BlockId argv2id(char *str)
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
	int ret;
	char *buffer;
	struct BlockId id;
	FILE *fp, *fid;

	if(argc < 2) {
		printf("args error: ./ssd_read_id <ID.txt> [dump file] \n");
		return -1;
	}

	if(3 == argc) {
		fp = fopen(argv[2], "ab");
		if(!fp) {
			printf("open file[%s] error\n", argv[2]);
			return -1;
		}
                fid = fopen(argv[1], "r");
		if(!fid) {
			printf("open file[%s] error\n", argv[1]);
                        fclose(fp);
			return -1;
		}
	}

	buffer = (char *)malloc(SIZE);
	if(!buffer) {
		printf("malloc error\n");
		ret = -1;
		goto CLOSE;
	}

	memset(buffer, 0, SIZE);
	/*
	ret = SSD_open(0);
	if(ret < 0) {
		printf("open error: %d\n", ret);
		goto CLOSE_SSD;
	}
	*/
        char *s;
        s = malloc(129);
        //fgets(s, 129, fid);
        //printf("%s\n",s);
        
        while(fgets(s,129,fid) != NULL) {
            id = argv2id(s);
            ret = SSD_read(0x10000, id, buffer, 8*1024, 0);	
	    if(ret != 0) 
            {
                printf("read ret: %d\n", ret);
                goto CLOSE_SSD;
            }
            fprintf(fp,"%d\n",*(int *)buffer);
            //fgets(s, 129, fid);
        }

CLOSE_SSD:
//	SSD_close_force(0);
	free(buffer);
        free(s);
CLOSE:
	fclose(fid);
	fclose(fp);
	return ret;
}
