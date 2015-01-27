import time
import os
import threading
import signal

def handler(signum, frame):
    print "stop waiting for"
    raise exception("timeout error")

def itime(N):
    i = 0
    try:
        while i <= N:
            print i
            i += 1
            time.sleep(1)
    except:
        raise Exception("timeout happened itime")

def die():
    raise Exception("timeout") 


try:
    print "begin test"
    
    main_pid = os.getpid()
    t1 = threading.Thread(target=itime, args=(10,))
    t2 = threading.Thread(target=run2, args=(20,))
    signal.signal(signal.SIGALRM, handler)
    signal.alarm(5)

    itime(10)
    signal.alarm(0)
except:
    raise Exception("timeout happened")


