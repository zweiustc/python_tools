#zweiustc@gmail.com
#!/bin/python
import json
import os
import logging
import pika
import sys
import threading
import time

logging.basicConfig(level=logging.WARNING,
                    format='%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s',
                    datefmt='%a, %d %b %Y %H:%M:%S',
                    filename='rabbit_consume.log')
                   # filemode='w+')

MESSAGES_PER_SECOND = 1
QUEUES_PER_HOST = 40
SECONDS_PER_ROUND = 60
HOST_NUM = 80

class HostConsume(object):

    def __init__(self):#, host):
        self.message_num = 0
        self.avg_latency = 0
        #self.host = host

    def run_consume(self, queue, exchange):
        logging.warning(('start queue send: %s') % queue)
        credentials = pika.PlainCredentials('rabbitclient', 'test')
        parameters = pika.ConnectionParameters('192.168.0.1',5672,'/',credentials)
        connection = pika.BlockingConnection(parameters)
        channel = connection.channel()
        channel.queue_declare(queue=queue,
                              durable=True,
                              exclusive=False,
                              auto_delete=False)

        avg_latency = 0.0
        message_cnt = 0
        total_num =  MESSAGES_PER_SECOND * SECONDS_PER_ROUND
        for method_frame, properties, body in channel.consume(queue):#, no_ack=True):
            try:
                message_cnt += 1
            	message = json.loads(body)
            	if 'timestamp' in message:
            	    timestamp = message['timestamp']
            	    latency = time.time() - timestamp
                    avg_latency += latency
                    avg_latency = avg_latency/2
            except Exception:
                logging.error('failed to calculate the latency %s') % message
                pass

            # Acknowledge the message
            channel.basic_ack(method_frame.delivery_tag)

            # Escape out of the loop after 10 messages
            test_round = SECONDS_PER_ROUND * MESSAGES_PER_SECOND
            if message_cnt % test_round == 0: 
                logging.warning(('queue %s latency: %s') % (queue, avg_latency))
                logging.warning(('current message %s and expect %s') % (message_cnt, total_num))
                
            if os.path.exists('./stop'):
                break
            #if message_cnt >= total_num:
            #    break

        logging.warning(('Test HOST num: %s') % HOST_NUM)
        logging.error(('queue %s the average latency: %s') % (queue, avg_latency))
        logging.warning(('queue %s total consume message %s and expect %s') % (queue, message_cnt,
                       total_num))
        print ('queue %s the average latency: %s') % (queue, avg_latency)
        print ('total consume message %s and expect %s') % (message_cnt, total_num)
        # Cancel the consumer and return any pending messages
        requeued_messages = channel.cancel()

        # Close the channel and the connection
        channel.close()
        connection.close()

    def run(self):
        print "QUEUES_PER_HOST: %s" % QUEUES_PER_HOST
        for i in range(HOST_NUM):
            for j in range(QUEUES_PER_HOST):
                queue_name = 'compute' + str(i) + '_queue_' + str(j)
                print "queuename: %s" % queue_name
                task = threading.Thread(target=self.run_consume, args=(queue_name, ''))
                #task.setDaemon(True)
                task.start()
                #task.join()


if __name__ == '__main__':
    #hostname = sys.argv[1]
    consume = HostConsume()
    consume.run()
