from mysql import Mysql
import sys

ROUND = 10

if __name__ == '__main__':
    # Init the connection to instance
    instance_db_test = Mysql(database="nova")
    instance_db_test.disconnect()

    # insert the data
    for i in range(ROUND):
        instance_db_test.reconnect()
        num = instance_db_test.get_instance_num()

        instance_db_test.reconnect()
        instance_db_test.insert_instance(count=10)

    # search from the data and print the latency
    instance_db_test.reconnect()
    instance_db_test.get_instance(
        instance_id='87d3e276-2efe-4ae5-9c62-06b68c06fe4b')

    # clear the fake data in the database
    instance_db_test.reconnect()
    instance_db_test.delete_instance()
    
    # Init the connection to volumes
    volume_db_test = Mysql(database="cinder")
    volume_db_test.disconnect()

    # insert the data
    for i in range(ROUND):
        volume_db_test.reconnect()
        num = volume_db_test.get_volume_num()

        volume_db_test.reconnect()
        volume_db_test.insert_volume(count=10)

    # search from the data and print the latency
    volume_db_test.reconnect()
    volume_db_test.get_volume(
        volume_id='87d3e276-2efe-4ae5-9c62-06b68c06fe4b')

    # clear the fake data in the database
    volume_db_test.reconnect()
    volume_db_test.delete_volume()
    
    # Init the connection to network ports
    port_db_test = Mysql(database="neutron")
    port_db_test.disconnect()

    # insert the data
    for i in range(ROUND):
        port_db_test.reconnect()
        num = port_db_test.get_port_num()

        port_db_test.reconnect()
        port_db_test.insert_port(count=10)

    # search from the data and print the latency
    port_db_test.reconnect()
    port_db_test.get_port(
        port_id='87d3e276-2efe-4ae5-9c62-06b68c06fe4b')

    # clear the fake data in the database
    port_db_test.reconnect()
    port_db_test.delete_port()
    
