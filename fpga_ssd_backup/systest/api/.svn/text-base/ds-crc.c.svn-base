#include <stdlib.h>
#include <stdio.h>
#include "bhp-lib.h"




int main(int argc, char * argv[])
{
	int buf_len = 1024*1024*1024;
	char *s = (char *)malloc(buf_len);
	int max_len = 1024*1024*10 +1;
	unsigned long long *table_k = (unsigned long long *)malloc(8*max_len);
	unsigned long long table[256];
	int i,j,N;
	unsigned long long crc0_t, crc1_t, k0, k1, a,b, poly,k2;
	//table_k[0] =0;
	k0 = 0x1LL;
	/*
	for(i=0;i<= 1024; i++){
 	//	k0 = mod2(0x1LL, POL, 33, i*8);
		if(i ==0 )
			k0 = mod2_step1(k0,POL,33,0);
		else
			k0 = mod2_step1(k0,POL,33,8);
		table_k[i] = k0;
		//printf("%llx\n",k0);
	}

	for(i=0; i<= 1024; i++){
		k0 = table_k[i];
		k0 = mod2_step2(k0);
		table_k[i] = k0;
		//printf("%llx\n",k0);
	}

	printf("start k diff...\n");
	for(i=0; i<= 1024; i++){
		k0 = mod2(0x1LL, POL, 33, i*8);
		//printf("%llx\n",k0);
		if(k0 != table_k[i])
			printf("i=%d, %llx, %llx\n",i,k0,table_k[i]);
	}
		
	return 0;
	*/
	gen_k(table_k , max_len);
	struct timeval start,end;
	double time=0;
	N = atoi(argv[1]);
	for(i=0; i < buf_len ; i++)
		s[i] = rand()%256;
	int crc, crc0, crc1, crc2, crc_init;
	//start 1KB try....
	crc = 0;//0xffffffff;
	gettimeofday(&start,NULL);
	for(i=0;i<16;i++){
	crc = baidu_crc32_qw( s,  crc, 6144*300 );
	}
	gettimeofday(&end,NULL);
	time = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
	//printf(" no-pipeline is %f MB/s\n",((double)(N)*3/1048576)/(time/1000000));
	printf(" no-pipeline is %f MB/s\n",((double)(6144*16*300)/1048576)/(time/1000000));
	printf("crc = %0x\n", crc);
	unsigned long long table1[256];
	poly = POL;
	k2 = mod2(0x1LL, poly, 33, 1024*8);
	printf("start pipeline...\n");
	gen_table(table1, k2);
	crc =0;
	gettimeofday(&start,NULL);
	for(i=0;i<16;i++){
	crc = baidu_crc32_fast( s,  crc, 6144*300 ,table1);
	}
	gettimeofday(&end,NULL);
	time = (end.tv_sec - start.tv_sec)*1000000 + (end.tv_usec - start.tv_usec);
	//printf(" no-pipeline is %f MB/s\n",((double)(N)*3/1048576)/(time/1000000));
	printf(" pipeline is %f MB/s\n",((double)(6144*16*300)/1048576)/(time/1000000));
	printf("crc = %0x\n", crc);
	crc_init = 0;
	poly = POL;
	//crc0 = baidu_crc32_qw(s,crc_init, 8);
	char *sp = s;
	crc0=0; crc1=0; crc2=0;
	crc = baidu_crc32_fast(s,0, N*3,table1);
	crc0 = baidu_crc32_qw(sp, crc0, N);
	crc1 = baidu_crc32_qw(sp+N, 0, N);
	crc2 = baidu_crc32_qw(sp+N*2, 0, N);
	a = (unsigned long long )(crc0) & 0xffffffffLL;
	k1 = mod2(0x1LL, poly, 33, N*8);	
	k0 = mod2(0x1LL, poly, 33, N*8*2);	
	//u0 = 0x08f158014LL;
	//u1 = 0x0e417f38aLL;
	crc0_t = pclmul(a, k0);
	a = (unsigned long long )(crc1) & 0xffffffffLL;
	crc1_t = pclmul(a, k1);
	//crc2 = baidu_crc32_qw(sp+336*3, crc2, 8);
	crc0 = baidu_crc32_qw((char *)(&crc0_t), 0, 8);
	crc1 = baidu_crc32_qw((char *)(&crc1_t), 0, 8);
	crc2 = crc2 ^ crc0;
	crc2 = crc2 ^ crc1;
	printf("crc2 = %0x\n", crc2);
	if(crc2 == crc)
		printf("CRC OK!\n");
	else
		printf("CRC DIFF!\n");

	printf("try new interface...\n");
	crc = 0;
//	crc = baidu_crc32_qw( s,  crc, N*7 );	
	int crc3, crc4,crc5,crc6;
	int crc_total;

	for(i=0; i<1024*1000*1024; i++){
	N = rand()% 1024*17+7;
	printf("N=%d\n",N);
	sp =s;
	crc0 = baidu_crc32_qw( sp,  0, N );
	sp = sp +N;
	crc1 = baidu_crc32_qw( sp,  0, N );
	crc = baidu_crc32_fast( s,  0, N*2 ,table1);
	//crc_total = MergeChecksum(crc0, crc1, N, table);
	crc_total = MergeChecksum_lut(crc0, crc1, N, table,table_k);
	int cmp;
	cmp = crc_total == crc ? 1:0;
	//printf("crc = %0x, crc_total = %0x, cmp = %d\n", crc, crc_total,cmp);
	if(cmp == 0) printf("DIFF\n");
	sp = sp +N;
	crc2 = baidu_crc32_qw( sp,  0, N );
	crc = baidu_crc32_fast( s,  0, N*3,table1 );
	//crc_total = MergeChecksum(crc_total, crc2, N, table);
	crc_total = MergeChecksum_lut(crc_total, crc2, N, table,table_k);
	cmp = crc_total == crc ? 1:0;
	//printf("crc = %0x, crc_total = %0x, cmp = %d\n", crc, crc_total,cmp);
	if(cmp == 0) printf("DIFF\n");
	sp = sp +N;
	crc3 = baidu_crc32_qw( sp,  0, N );
	crc = baidu_crc32_fast( s,  0, N*4 ,table1);
	//crc_total = MergeChecksum(crc_total, crc3, N,table);
	crc_total = MergeChecksum_lut(crc_total, crc3, N,table, table_k);
	cmp = crc_total == crc ? 1:0;
	//printf("crc = %0x, crc_total = %0x, cmp = %d\n", crc, crc_total,cmp);
	if(cmp == 0) printf("DIFF\n");
	sp = sp +N;
	crc4 = baidu_crc32_qw( sp,  0, N );
	crc = baidu_crc32_fast( s,  0, N*5 ,table1);
	//crc_total = MergeChecksum(crc_total, crc4, N,table);
	crc_total = MergeChecksum_lut(crc_total, crc4, N,table, table_k);
	cmp = crc_total == crc ? 1:0;
	//printf("crc = %0x, crc_total = %0x, cmp = %d\n", crc, crc_total,cmp);
	if(cmp == 0) printf("DIFF\n");
	sp = sp +N;
	crc5 = baidu_crc32_qw( sp,  0, N );
	crc = baidu_crc32_fast( s,  0, N*6 ,table1);
	//crc_total = MergeChecksum(crc_total, crc5, N, table);
	crc_total = MergeChecksum_lut(crc_total, crc5, N, table, table_k);
	cmp = crc_total == crc ? 1:0;
	//printf("crc = %0x, crc_total = %0x, cmp = %d\n", crc, crc_total,cmp);
	if(cmp == 0) printf("DIFF\n");
	sp = sp +N;
	crc6 = baidu_crc32_qw( sp,  0, N );
	crc = baidu_crc32_fast( s,  0, N*7, table1 );
	/*
	crc_total = MergeChecksum(crc0, crc1, N, table);
	crc_total = MergeChecksum(crc_total, crc2, N, table);
	crc_total = MergeChecksum(crc_total, crc3, N, table);
	crc_total = MergeChecksum(crc_total, crc4, N, table);
	crc_total = MergeChecksum(crc_total, crc5, N, table);
	crc_total = MergeChecksum(crc_total, crc6, N, table);
	*/

	crc_total = MergeChecksum_lut(crc0, crc1, N, table, table_k);
	crc_total = MergeChecksum_lut(crc_total, crc2, N, table, table_k);
	crc_total = MergeChecksum_lut(crc_total, crc3, N, table, table_k);
	crc_total = MergeChecksum_lut(crc_total, crc4, N, table, table_k);
	crc_total = MergeChecksum_lut(crc_total, crc5, N, table, table_k);
	crc_total = MergeChecksum_lut(crc_total, crc6, N, table, table_k);
	//printf("crc = %0x, crc_total = %0x\n", crc, crc_total);
	if(crc_total == crc)
		printf("CRC OK!\n");
	else
		printf("CRC DIFF!\n");
	}
	//test ds api
	int *crc_table = (int *)malloc(N*10*4);
	printf("start test crc_fast...\n");
	for(i=0; i<1024*1024*1024; i++){
		crc = baidu_crc32_qw( s,  0, i );
		crc0 = baidu_crc32_fast( s,  0, i ,table1);
		if(crc != crc0)
			printf("fast != qw\n");
		crc_table[i] = crc0;
	}
	printf("start test merge...\n");
	for(i=2;i<N*10;i++){
	}

}
