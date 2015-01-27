LIB=../../api/output/lib
INCLUDE=../../api/output/include
FLAGS=ssdapi
CFG_FLASH_N=cfg_flash_n
CFG_S6_N_40=cfg_s6_n_40
CFG_V5_N=cfg_v5
RD_N=rd_n
WR_N=wr_n

default:
	gcc cfg_flash.c -o ${CFG_FLASH_N} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc cfg_s6-400000.c -o ${CFG_S6_N_40} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc cfg_v5.c -o ${CFG_V5_N} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc rd_reg_test.c -o ${RD_N} -I${INCLUDE} -L${LIB} -l${FLAGS}
	gcc wr_reg_test.c -o ${WR_N} -I${INCLUDE} -L${LIB} -l${FLAGS}
clean:
	rm -f ${CFG_FLASH_N}
	rm -f ${CFG_S6_N_40}
	rm -f ${CFG_V5_N}
	rm -f ${RD_N}
	rm -f ${WR_N}
