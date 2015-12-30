#!/usr/bin/python
import logging
import time 
import eventlet
from eventlet import greenthread
from eventlet import tpool
from eventlet.timeout import Timeout

eventlet.monkey_patch()

logging.basicConfig(level=logging.WARNING,
                    format='%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s',
                    datefmt='%a, %d %b %Y %H:%M:%S',
                    filename='threadevent.log')

class test_event(object):

    datefmt='%a, %d %b %Y %H:%M:%S'
    interval = 1

    def date_output(self, content="test", event=None):
        for i in range(10):
            cur_time = time.strftime(self.datefmt, time.localtime(time.time()))
            logging.warning(("Wait and sleep for %s seconds in round %s") % (self.interval, i))
            # time.sleep or greenthread sleep make the current coroutine give the time slice
            #time.sleep(self.interval)
            eventlet.greenthread.sleep(0)
            logging.warning(("Output content: %s at timestamp: %s in round %s") % (content, cur_time, i))
        if event is not None:
            result = event.wait() 
            logging.warning(("the result of event is %s") % result)

    def start_to_output(self, content, event=None): 
        # the two way of spawn equals
        opthread = greenthread.spawn(self.date_output, content, event=event)
        #opthread = eventlet.spawn(self.date_output, content)
        return opthread


if __name__ == "__main__":
    test = test_event()
    def thread_finished(thread, event, name=None):
        logging.warning(("Finish the thread"))
        event.send(name)

    with Timeout(30, False):
        finish_event = eventlet.event.Event()

        opthread = test.start_to_output('test event content')
        opthread2 = test.start_to_output('test event content 2', finish_event)

        #add the callback func
        opthread.link(thread_finished, finish_event, 'opthread')

        # the greanthread wait will block the thread until the coroutine end
        logging.warning(("wait the thread one"))
        opthread.wait()
        logging.warning(("wait the thread two"))
        opthread2.wait()

