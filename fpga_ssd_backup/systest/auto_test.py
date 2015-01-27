#!/usr/bin/python
# Filename : auto_test.py designed for SSD testing automatically
# Warning : the output of auto_test.py will recovery its previous

import os
import shutil
import string

# current directionary, work dic
cur_dir  = os.getcwd()
# output dictionary path
output_func   = cur_dir + '/output_func'
output_perf   = cur_dir + '/output_perf'
output_pres   = cur_dir + '/output_pres'
# logs dictionary containing all logs
logs     = cur_dir + '/logs'
# test.log recording final test result
test_log =  logs + '/test.log'
# results.log recording the output of the testing programs
res_log  =  logs + '/results.log'

#cases written by c
cases_cc   = [];
cases_perf = [];
cases_pres = [];

# total cases
total_cases = 0
fail_cases  = 0

#clear log files
try: 
	shutil.rmtree(logs)
except OSError:
	print "logs dictionary does not exist" 

try:
	os.mkdir(logs)
except OSError:
	print "can not mkdir logs dictionary"

# if there is no output, run Makefile
#if not os.path.exists(output_func):
#	if os.system("make") != 0:
#		print "can not make the Makefile"
#		exit(-1)
#	print "Makefile runs successfully"

# gen the case list
for fname in os.listdir(output_func):
	total_cases += 1
	cases_cc.append(fname)

for fname in os.listdir(output_pres):
        total_cases += 1
        cases_pres.append(fname)

for fname in os.listdir(output_perf):
        total_cases += 1
        cases_perf.append(fname)


cases_new=[];
for case in cases_cc:
	if case[:4]=="test":
		cases_new.append(case)

#the first time
#if os.system("./output_func/open_newcard >> res_log") == 0:
#	print "open_newcard runs successfully" % open_newcard
#else:
#	print "open_newcard can not run" % open_newcard



for case in cases_new:
	if os.system("./output_func/%s >> %s" % (case,'log_'+case)) == 0:
		print "%s runs successfully" % case
	else:
		print "%s can not run" % case
	
	if os.system("./output_func/clear_all >>%s" % ('log_'+case)) == 0:
		print " clear_all runs successfully"
	else:
		print " clear_all can not run"


for case in cases_perf:
        if os.system("./output_perf/%s >> %s" % (case,'log_'+case)) == 0:
                print "%s runs successfully" % case
        else:
                print "%s can not run" % case

        if os.system("./output_func/clear_all >>%s" % ('log_'+case)) == 0:
                print " clear_all runs successfully"
        else:
                print " clear_all can not run"
		


try:
	test_file = open(test_log, 'w+')
	test_file.write('Begin to check the test result \n')
	
	for case in cases_new:
		res_file  = open('log_'+case, 'r')
		#lines = res_file.readlines()
		#for line in res_file:
		#	if line.find("ERROR") != -1:
		#		test_file.write( "%3s is failed\n" % (case));
		#		break
		#	else:
		#		test_file.write( "%3s is passed\n" % (case));

		if(case == "log_test_close_write_read_delete_multi")\
                or(case == "log_test_multi_channel_delete_one_id")\
                or(case == "log_test_multi_channel_read_delete_one_id")\
                or(case == "log_test_multi_channel_write_delete_one_id")\
                or(case == "log_test_multi_channel_write_one_id" ):
                        for line in res_file:
                                if line.find("ERROR") == 1:
                                        test_file.write( "%10s is failed\n" % (case));
                                        break
                else:
                        for line in res_file:
                                if line.find("ERROR") != -1:
                                        test_file.write( "%10s is failed\n" % (case));
                                        break

finally:
	test_file.write('end to check the test result \n')
	res_file.close()
	test_file.close()

# end of file
