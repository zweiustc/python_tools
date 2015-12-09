import json
import logging
import pika
import sys
import threading
import time

logging.basicConfig(level=logging.WARNING,
                    format='%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s',
                    datefmt='%a, %d %b %Y %H:%M:%S',
                    filename='rabbit_publish.log')
MESSAGES_PER_SECOND = 1
QUEUES_PER_HOST = 40
SECONDS_PER_ROUND = 60

class HostPublish(object):

    def __init__(self, host, queue=None, queue_num=0,
                 exchange='', routing_key=''):
        self.queue_num = queue_num
        self.host = host
        self.queue = queue
        self.exchange = exchange
        self.routing_key = routing_key
        self.stop = False
        self.threads = list()

    def run_compute(self, queue_num, queue, exchange):

        try:
            curr_queue_num = queue_num or self.queue_num
            curr_queue = queue or self.queue
            curr_exchange = exchange

            credentials = pika.PlainCredentials('rabbitclient', 'test')
            parameters = pika.ConnectionParameters('192.169.0.1',5672,'/',credentials)
            connection = pika.BlockingConnection(parameters)
            channel = connection.channel()
            interval = 1.0 / MESSAGES_PER_SECOND
        except Exception as err:
            logging.error(('Failed to connect to rabbitmq for %s') % str(err))
        for i in range(MESSAGES_PER_SECOND * SECONDS_PER_ROUND):
            try:
                message = {'name': 'zhangwei pressure test of rabbit',
                           'queue_num': curr_queue_num,
                           'host': self.host,
                           'timestamp': time.time()}
                channel.basic_publish(exchange=curr_exchange,    
                                      routing_key=curr_queue,    
                                      #routing_key='testzhangwei',    
                                      body=json.dumps(message))    
                time.sleep(interval)
            except Exception as err:
                logging.error(('Failed to publish message %s for %s') % (message, err))
                pass
        connection.close()   

    def run(self):
        for i in range(QUEUES_PER_HOST):
            queue_name = self.host + '_queue_' + str(i)
            task = threading.Thread(target=self.run_compute, args=(i, queue_name, ''))
            #task.setDaemon(True)
            task.start()
            #task.join()
            self.threads.append(task)


if __name__ == '__main__':            
    hostname = sys.argv[1]
    host1 = HostPublish(hostname)
    host1.run()
