#include <stdio.h>

char *error_string[] = {
	"SSD_OK",
	"SSD_ERR_START_PAGE_ALIGN",
	"SSD_ERR_LEN_PAGE_ALIGN",
    "SSD_ERR_START_BLOCK_ALIGN",
    "SSD_ERR_LEN_BLOCK_ALIGN",
	"SSD_ERR_PARAM_OVERFLOW",
	"SSD_ERR_PARAM_WRLEN",
	"SSD_ERR_NULL",
	"SSD_ERR_NOMEM",
	"SSD_ERR_TIMEOUT",
	"SSD_ERR_NO_DEV",
	"SSD_ERR_NOT_SSD",
	"SSD_ERR_DMA_MAP",
	"SSD_ERR_DEL_TIMER",
	"SSD_ERR_COPY_FROM_USER",
	"SSD_ERR_COPY_TO_USER",
	"SSD_ERR_CHANNEL",
	"SSD_ERR_SSD",
	"SSD_ERR_WAIT",
	"SSD_ERR_BAD_BLOCK",
	"SSD_ERR_NO_INVALID_BLOCKS",
	"SSD_ERR_TABLE_TYPE",
	"SSD_ERR_OEPN_DEV_FAILED",
	"SSD_ERR_NO_VALID_FD",
	"SSD_ERR_NOT_OPENED",
	"SSD_ERR_NOT_CLOSED",
	"SSD_ERR_ALLOC_BLOCK",
	"SSD_ID_NO_EXIST",
	"SSD_ID_ALREADY_EXIST",
	"SSD_ID_MAP_ERROR",
	"SSD_ERR_NO_VALID_RESERVE",
	"SSD_ERR_META_BLOCK_ALIGN",
	"ENOMETA",
	"EREADFLUSH",
	"EMETAUNVAL",
	"EIDUNVAL",
	"ETABLEDATA"
};

int main(int argc, char *argv[])
{
	int error_no;
	
	if(argc != 2) {
		printf("%s <error_no>\n", argv[0]);
		return -1;
	}

	error_no = abs(atoi(argv[1]));
	if(error_no > 36) {
		printf("error_no [0, 36]\n");
		return -1;
	}

	printf("error_no[%d]: %s\n", error_no, error_string[error_no]);

	return 0;
}
