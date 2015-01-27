import thread
import threading
import os

def run1(n):
    run1_pid = os.getpid()
    print "run1_pid %s" % run1_pid
    for i in range(n):
        print i
        
def run2(n):
    run2_pid = os.getpid()
    print "run2_pid %s" % run2_pid
    for i in range(n):
        print i

print "begin test"
main_pid = os.getpid()
print "main_pid %s"%main_pid
#thread.start_new_thread(run1, (10,))
#thread.start_new_thread(run2, (10,))
#thread.start_new_thread(run2, (10,))
t1 = threading.Thread(target=run1, args=(10,))
t1.start()
t2 = threading.Thread(target=run2, args=(20,))
t2.start()
