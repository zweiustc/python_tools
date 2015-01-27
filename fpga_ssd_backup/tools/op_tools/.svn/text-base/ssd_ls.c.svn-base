#include <sys/types.h>
#include <unistd.h>
#include "ssd_api.h"

int get_all_file(const char *fid, const struct stat *st, int flag)
{
    printf("%s\n", fid);
    return 0;
}

int get_channel_file(const char *fid, const struct stat *st, int flag)
{
	return 0;
}

int get_all_file_detail(const char *fid, const struct stat *st, int flag)
{
	/*ID	logicblock	channel*/
	printf("%s\t%lu\t%lu\n", fid, st->st_ino, st->st_rdev);
	return 0;
}

int get_channel_file_detail(const char *fid, const struct stat *st, int flag)
{
	return 0;
}

int main(int argc, char *argv[])
{
    int ret, detail = 0;
	int c;
     
	while((c = getopt(argc, argv, "lh")) > 0) {
		switch(c) {
			case 'h':
				printf("%s -l[detail]\n", argv[0]);
				return 0;

			case 'l':
				detail = 1;
				break;

			defalt:
				printf("args error\n");
				return -1;
		}
	}

	if(!detail) {
		printf("fileID\n");
		ret = SSD_ftw(MONITOR_DEV, get_all_file, 0);
	} else {
		printf("fileID\tlogic_block\tchannel\n");
		ret = SSD_ftw(MONITOR_DEV, get_all_file_detail, 0);
	}
    
    if (ret != SSD_OK)
       printf("ERROR! ret is %d\n", ret);

    return 0;
}
