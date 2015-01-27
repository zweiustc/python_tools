#!/usr/bin/env python
'''
     @server.py        :  used for receive cmd from other compu site
     @function         :  used for receive cmd from other compu site
                          eg:  call shell script to kill unitserver and close ssd
     @version          :  1.0.0.1
     @author           :  zhangwei<zhangwei29@baidu.com>
     @create           :  2013-10-25
'''

import socket
import os,sys
import commands

'''
get the current hostname for binding the socket
'''
status, out = commands.getstatusoutput('hostname')
if status == 0:
    servername = out
else:
    print  "error: failed to get hostname!\n"

'''
bind the socket and listen
'''
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.bind((servername, 9001))
sock.listen(5)

'''
get cmd from client and execution the shell script
then send the result to client
'''
while True:
    connection,address = sock.accept()
    try:
        connection.settimeout(5)
        buf = connection.recv(1024)
        print buf
        fd = os.popen(buf)
        output = fd.read()
    #    print output
        connection.send(output)
    except socket.timeout:
        print 'time out'
    connection.close()
