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
#if os.system("./output_func/output/open_newcard >> res_log") == 0:
#	print "open_newcard runs successfully" % open_newcard
#else:
#	print "open_newcard can not run" % open_newcard


for i in range(1,1000):
	for case in cases_pres:
		if os.system("./output_pres/%s >> %s" % (case,'log_'+case)) == 0:
			print "%s runs successfully" % case
		else:
			print "%s can not run" % case
	
		if os.system("./output_func/clear_all >>%s" % ('log_'+case)) == 0:
			print " clear_all runs successfully"
		else:
			print " clear_all can not run"




# end of file
