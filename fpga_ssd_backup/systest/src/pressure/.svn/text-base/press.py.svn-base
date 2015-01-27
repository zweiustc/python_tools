#!/usr/bin/python
# Filename : auto_test.py designed for SSD testing automatically
# Warning : the output of auto_test.py will recovery its previous

import os
import shutil

# current directionary, work dic
cur_dir  = os.getcwd()
# output dictionary path
output   = cur_dir + '/output'
# logs dictionary containing all logs
logs     = cur_dir + '/logs'
# test.log recording final test result
test_log =  logs + '/test.log'
log_1    =  logs + '/log_1.log'
log_2    =  logs + '/log_2.log'
log_3    =  logs + '/log_3.log'
log_4    =  logs + '/log_4.log'
log_5    =  logs + '/log_5.log'
log_6    =  logs + '/log_6.log'
# results.log recording the output of the testing programs
res_log  =  logs + '/results.log'

#cases written by c
cases_cc = [];
# total cases
total_cases = 0
fail_cases  = 0

# shell print with color
def color(s, succ=True):
	if succ:
		string = s + 'is PASS!!!'
		s = string.center(40, "*")
		return "%s[32;2m%s%s[0m" % (chr(27), s, chr(27)) #green
	else:
		string = s + 'is FAIL!!!'
		s = string.center(40, "*")
		return "%s[31;2m%s%s[0m" % (chr(27), s, chr(27)) #red



#clear log files
try: 
	shutil.rmtree(logs)
except OSError:
	print "logs dictionary does not exist" 

try:
	os.mkdir(logs)
except OSError:
	print "can not mkdir logs dictionary"



#clear the whole ssd
if os.system("./../clear_all >> res_log") == 0:
	print "open_newcard runs successfully"
else:
	print "open_newcard can not run"




for i in range(1,100):
	if os.system("./test_multi_channel_all_operation >> log_1") == 0:
		print "test_multi_channel_all_operation runs successfully"
	else:
		print "test_multi_channel_all_operation can not run" 
	
	if os.system("./../clear_all >> res_log_1") == 0:
		print "clear_all runs successfully" 
	else:
		print "clear_all can not run"
	
	if os.system("./test_multi_channel_all_operation_1 >> log_2") == 0:
                print "test_multi_channel_all_operation_1 runs successfully"
        else:
                print "test_multi_channel_all_operation_1 can not run"

        if os.system("./../clear_all >> res_log_2") == 0:
                print "clear_all runs successfully" 
        else:
                print "clear_all can not run"

	if os.system("./test_multi_channel_all_operation_2 >> log_3") == 0:
                print "test_multi_channel_all_operation_2 runs successfully"
        else:
                print "test_multi_channel_all_operation_2 can not run"

        if os.system("./../clear_all >> res_log_3") == 0:
                print "clear_all runs successfully" 
        else:
                print "clear_all can not run"

	if os.system("./test_multi_channel_all_operation_3 >> log_4") == 0:
                print "test_multi_channel_all_operation_3 runs successfully"
        else:
                print "test_multi_channel_all_operation_3 can not run"

        if os.system("./../clear_all >> res_log_4") == 0:
                print "clear_all runs successfully" 
        else:
                print "clear_all can not run"
	
	for j in range(1,10)
		if os.system("./test_multi_channel_full_channel_compare >> log_5") == 0:
                	print "test_multi_channel_full_channel_compare_1 runs successfully"
        	else:
                	print "test_multi_channel_full_channel_compare_1 can not run"

        if os.system("./../clear_all >> res_log_5") == 0:
                print "clear_all runs successfully" 
        else:
                print "clear_all can not run"

	if os.system("./test_multi_channel_write_read_delete_norandab >> log_6") == 0:
                print "test_multi_channel_write_read_delete_norandab runs successfully"
        else:
                print "test_multi_channel_write_read_delete_norandab can not run"
	
	if os.system("./../clear_all >> res_log_6") == 0:
                print "clear_all runs successfully"
        else:
                print "clear_all can not run"

print "the test is over"
    #    if os.system("./../clear_all_tmp >> res_log") == 0:
    #            print "clear_all runs successfully" 
    #    else:
    #            print "clear_all can not run"

#try:
#	test_file = file(test_log, 'a')
#	print >> test_file, "caseName\tResults"

#	res_file  = file(res_log, 'r')
	#lines = res_file.readlines()
#	for line in res_file:
#		if line.find("SUCC") != -1:
#			print >> test_file, "%3s\t%s" % (line.strip().split()[2], line.strip().split()[-1])
#			print color(line.strip().split()[2])
#		elif line.find("FAIL") != -1:
#			print >> test_file, "%3s\t%s" % (line.strip().split()[2], line.strip().split()[-1])
#			print color(line.strip().split()[2])
#finally:
#	res_file.close()
#	test_file.close()

# end of file
