#!/usr/bin/env python
'''
     @server.py        :  used for send cmd to the target server
     @function         :  used for send cmd to the target server
                          eg:  send cmd to the server && the server will 
                               call shell script to kill unitserver and close ssd
     @version          :  1.0.0.1
     @author           :  zhangwei<zhangwei29@baidu.com>
     @create           :  2013-10-25
'''

import socket
import sys

'''
judge the input && show the usage
'''
if len(sys.argv) < 3:
    print "Usage:python %s path&&shellscript hostname\n\
       ex: Usage:python %s /home/op_tools/test.sh cq01-zhangwei-longtime.vm.baidu.com" % (sys.argv[0],sys.argv[0])
    sys.exit(1)

'''
connectl to the server
'''    
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((sys.argv[2], 9001))

'''
recv the shell script cmd and send to the server
'''
cmd="sh %s" % (sys.argv[1])
sock.send(cmd)

'''
recv the respond && show the result
'''
print sock.recv(1024)
sock.close()
