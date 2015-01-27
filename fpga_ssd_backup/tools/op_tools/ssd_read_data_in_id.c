#include "ssd_api.h"
#define SIZE (10 * 1024 * 1024)


/*for crc*/
unsigned long long table[256];
unsigned long long key;

#define CRC_DATA 0x1LL
#define CRC_POLY 0x11edc6f41LL
#define CRC_DATA_BIT (1024 * 8)
#define CRC_POLY_BIT 33

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
        return id;
}




int main(int argc, char* argv[]) 
{
	int ret, c;
	char *buffer;
	char *buffer_content;
	struct BlockId id;
	int   flag_id = 0;
	int is_crc = 0;
	int crc_calcu;
	int32_t len = -1, len_fixed = -1;
	int32_t offset = -1, offset_fixed = -1;
	int32_t data_size;	
	FILE *fp = NULL;
	
	
	while((c = getopt(argc, argv, "i:l:o:f:hc")) > 0) {
		switch(c){
			case  'h':  //help
				printf("%s -i<ID> -l<length> -o<offset> -f<filename> -c(add crc)\n", argv[0]);
				goto CLOSE;
			case  'i':
				id = argv2id(optarg);  //get id
				flag_id = 1;
				break;
			case  'l':
				len = atoi(optarg);    //get length
				break;
			case  'o':
				offset = atoi(optarg);  //get offset
				break;
			case  'c':
				is_crc = 1;
				break;
			case  'f':
				fp = fopen(optarg, "ab");
				if(!fp) {
					printf("open file[%s] error\n", optarg);
					return -1;
				}
				break;
			default:
				printf("error parameter input\n");
				printf("%s -i<ID> -l<length> -o<offset> -f<filename> -c(add crc)\n", argv[0]);
				ret = -1;
				goto CLOSE;
		}
	}

	if((len < 0) || (offset < 0) || (flag_id == 0)) {	
		printf("%s -i<ID> -l<length> -o<offset> -f<filename> -c(add crc)\n", argv[0]);
		printf("id length and offset should not be less than zero \n");
		ret = SSD_ERR_INVALID_PARAM;
		goto CLOSE;
	}
	else if(0 == len) {
		ret = SSD_OK;
		goto CLOSE;
	}

	data_size = (id.m_nHigh & 7) == 0 ? BT_INDEX_SIZE : BT_DATA_SIZE;
	if((offset + len) > data_size) {
		ret = SSD_ERR_PARAM_OVERFLOW;
		goto  CLOSE;
	}
	
	
	/*for crc init */
	if(is_crc) {
		key = mod2(CRC_DATA, CRC_POLY, CRC_POLY_BIT, CRC_DATA_BIT);
		gen_table(table, key);
	}

	//fixed offset and length for reading pages
	offset_fixed = (offset/SSD_PAGE_SIZE)*SSD_PAGE_SIZE;
	len_fixed = offset%SSD_PAGE_SIZE + len ;
	len_fixed = ((len_fixed - 1) /SSD_PAGE_SIZE + 1) * SSD_PAGE_SIZE;

               
	buffer = (char *)malloc(len_fixed);
	if(!buffer) {
		printf("malloc error\n");
		ret = -1;
		goto CLOSE;
	}

	memset(buffer, 0, len_fixed);
	
	ret = SSD_read(MONITOR_DEV, id, buffer, len_fixed, offset_fixed);
	if(SSD_OK != ret)
		printf("read erorr: %d \n", ret);

	//fixed write start position
	buffer_content = buffer + offset % SSD_PAGE_SIZE ;
	
	if(NULL != fp)
		fwrite(buffer_content, len, 1, fp);
	else
		fwrite(buffer_content, len, 1, stdout);

	if(is_crc) {
		crc_calcu = baidu_crc32_fast(buffer_content, 0, len, table);
		printf("parameters of the data: len = %d  offset = %d crc = %x \n", len, offset, crc_calcu);
	}

	free(buffer);
CLOSE:
	if(NULL != fp)
		fclose(fp);
	return ret;
	
}
