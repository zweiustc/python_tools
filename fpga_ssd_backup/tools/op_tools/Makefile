LIB=../../api/output/lib
INCLUDE=../../api/output/include
FLAGS=ssdapi -g
TOOLS=ssd_read_id
READ_BYTES=ssd_read_data_in_id
READ_TABLE=ssd_read_table
SSD_LS=ssd_ls
OPEN=open
INIT_NEWCARD=init_newcard
ERASE_ALLDATA=erase_alldata
CLOSE_FORCE=close_force
SSD_CHECK_ALLID=ssd_checkcrc_allid
SSDSTAT=ssd_stat
UTIL_ERASETABLE=util_erasetable
ERROR=ssd_error
CHECK_ALL=ssd_check_all
CHECK_channel=ssd_check_channel
ERASE_ID=ssd_erase_id
MARK_BLOCK=ssd_mark_bad_block
ID2CHANNEL=ssd_id_to_channel
MARK_ERASE_COUNT=ssd_mark_erase_count
READ_FLUSH=ssd_read_flush
WR_RD_CHECK=ssd_wr_rd_check
ANALYSIS_FLUSH= analysis_flush_data
CHECK_ERR_CRC=check_err_crc
SSD_CHECK_CHANNEL=ssd_check_channel
FLUSH=ssd_flush
ERASE_ID_NOOPEN=ssd_erase_id_without_open
SSD_RECOVER=ssd_recover
SSD_CHECK_RECOVER=ssd_check_recover
default:
	gcc ssd_read_id.c -o ${TOOLS} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc ssd_read_data_in_id.c  -o ${READ_BYTES} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc ssd_read_table.c -o ${READ_TABLE} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc ssd_ls.c -o ${SSD_LS} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc open.c -o ${OPEN} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc init_newcard.c -o ${INIT_NEWCARD} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc erase_alldata.c -o ${ERASE_ALLDATA} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc close_force.c -o ${CLOSE_FORCE} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc ssd_checkcrc_allid.c -o ${SSD_CHECK_ALLID} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc ssd_stat.c -o ${SSDSTAT}
	gcc util_erasetable.c -o ${UTIL_ERASETABLE} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc ssd_error.c -o ${ERROR}
	gcc ssd_check_all.c -o ${CHECK_ALL} -I${INCLUDE} -L${LIB} -l${FLAGS} -lpthread
	gcc ssd_erase_id.c -o ${ERASE_ID} -I${INCLUDE} -L${LIB} -l${FLAGS} -lpthread
	gcc ssd_mark_bad_block.c -o ${MARK_BLOCK} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc ssd_id_to_channel.c -o ${ID2CHANNEL} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc ssd_mark_erase_count.c -o ${MARK_ERASE_COUNT} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc ssd_read_flush.c -o ${READ_FLUSH} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc ssd_wr_rd_check.c -o ${WR_RD_CHECK} -I${INCLUDE} -L${LIB} -l${FLAGS} -lpthread
	gcc analysis_flush_data.c -o ${ANALYSIS_FLUSH} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc check_err_crc.c -o ${CHECK_ERR_CRC} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc ssd_check_channel.c -o ${SSD_CHECK_CHANNEL} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc ssd_flush.c -o ${FLUSH} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc ssd_erase_id_without_open.c -o ${ERASE_ID_NOOPEN} -I${INCLUDE} -L${LIB} -l${FLAGS} -lpthread
	gcc ssd_recover.c -o ${SSD_RECOVER} -I${INCLUDE} -L${LIB} -l${FLAGS} -lpthread
	gcc ssd_check_recover.c -o ${SSD_CHECK_RECOVER} -I${INCLUDE} -L${LIB} -l${FLAGS} -lpthread
clean:
	rm -f ${TOOLS}
	rm -f ${READ_BYTES}
	rm -f ${READ_TABLE}
	rm -f ${SSD_LS}
	rm -f ${OPEN}
	rm -f ${INIT_NEWCARD}
	rm -f ${CLOSE_FORCE}
	rm -f ${SSD_CHECK_ALLID}
	rm -f ${SSDSTAT}
	rm -f ${UTIL_ERASETABLE}
	rm -f ${ERROR}
	rm -f ${CHECK_ALL}
	rm -f ${ERASE_ID}
	rm -f ${MARK_BLOCK}
	rm -f ${ID2CHANNEL}
	rm -f ${MARK_ERASE_COUNT}
	rm -f ${READ_FLUSH}
	rm -f ${WR_RD_CHECK} 
	rm -f ${ANALYSIS_FLUSH}
	rm -f ${CHECK_ERR_CRC}
	rm -f ${SSD_CHECK_CHANNEL}
	rm -f ${FLUSH}
	rm -f ${ERASE_ID_NOOPEN}
	rm -f ${SSD_RECOVER}
	rm -f ${SSD_CHECK_RECOVER}
