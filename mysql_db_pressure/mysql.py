import MySQLdb as mdb
import sys
import time
import uuid

class Mysql(object):
    def __init__(self, ip='10.168.91.1', user='root',
                 password='Passw0rd', database="nova"):
        self.ip = ip
        self.user = user
        self.password = password
        self.database = database
        self.db = mdb.connect(self.ip, self.user, self.password,
                              self.database)
        self.name = None
        self.cursor = self.db.cursor()

    def reconnect(self):
        self.db = mdb.connect(self.ip, self.user, self.password,
                              self.database)
        self.cursor = self.db.cursor()

    def disconnect(self):
        if self.db:
            self.db.close()

    def insert_instance(self, name='test_pressure_mysql',
                        count=100000):
        try:
            start_time = time.time()
            self.name = name
            for i in range(count):
                user_id = uuid.uuid4()
                project_id = user_id
                vm_state = 'active'
                memory_mb = 4096
                vcpus = 4
                hostname = name
                host = name
                reservation_id = 'r-f79v0p3a'
                display_name = name
                availability_zone = 'dev'
                root_gb = 40
                sql = "INSERT INTO instances(user_id, project_id, vm_state, \
                       memory_mb, vcpus, hostname, host, reservation_id, display_name, \
                       availability_zone, uuid, root_gb) VALUES ('%s', '%s', '%s', \
                       '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s')" % \
                       (user_id, project_id, vm_state, \
                       memory_mb, vcpus, hostname, host, reservation_id, display_name, \
                       availability_zone, uuid.uuid4(), root_gb)
                try:
                    self.cursor.execute(sql)
                    self.db.commit()
                except:
                    self.db.rollback()
            end_time = time.time()
            print "Insert %s item to instance cost: %s seconds" % \
                  (count, end_time - start_time)
        except:
            pass
        finally:
            self.db.close()

    def delete_instance(self, name='test_pressure_mysql'):
        delete_name = name or self.name
        try:
            # close the foreign key check
            sql = "SET FOREIGN_KEY_CHECKS=0"
            self.cursor.execute(sql)

            sql = "DELETE FROM instances WHERE hostname='%s'" % delete_name
            start_time = time.time()
            self.cursor.execute(sql)
            self.db.commit()
        except Exception as err:
            self.db.rollback()
        finally:
            end_time = time.time()
            print "Delete inserted data from instances cost: %s seconds" % \
                  (end_time - start_time)
            self.db.close()

    def get_instance_num(self, name="test_pressure_mysql"):
        get_name = name or self.name
        sql = "select count(id) from instances where hostname='%s'" % get_name
        result = None
        try:
            start_time = time.time()
            self.cursor.execute(sql)
            result = self.cursor.fetchall()
        except Exception as err:
            self.db.rollback()
        finally:
            end_time = time.time()
            print "Get intems num of instance cost: %s seconds" % \
                  (end_time - start_time)
            print "Current items num of instances database: %s" % result
            self.db.close()

    def get_instance(self, instance_id):
        sql = "select * from instances where id='%s'" % instance_id
        try:
            start_time = time.time()
            self.cursor.execute(sql)
            result = self.cursor.fetchall()
        except Exception as err:
            self.db.rollback()
        finally:
            end_time = time.time()
            print "Search instance %s cost: %s seconds" % \
                  (instance_id, end_time - start_time)
            self.db.close()

    def insert_volume(self, name='test_pressure_mysql',
                      count=100000):
        try:
            start_time = time.time()
            self.name = name
            for i in range(count):
                user_id = uuid.uuid4()
                id = user_id
                project_id = user_id
                host="controller1.ksc@cluster01@sheepdog01#sheepdog01"
                size=40
                availability_zone="dev"
                status="available"
                display_name=name
                provider_location="127.0.0.1:7000:sheepdog"

                sql = "INSERT INTO volumes(user_id, id, project_id, host, \
                       size, availability_zone, status, display_name, \
                       provider_location) VALUES ('%s', '%s', '%s', \
                       '%s', '%s', '%s', '%s', '%s', '%s')" % \
                       (user_id, id, project_id, host, size, availability_zone, \
                       status, display_name, provider_location)
                try:
                    self.cursor.execute(sql)
                    self.db.commit()
                except:
                    self.db.rollback()
            end_time = time.time()
            print "Insert %s item to volumes cost: %s seconds" % \
                  (count, end_time - start_time)
        except:
            pass
        finally:
            self.db.close()

    def delete_volume(self, name='test_pressure_mysql'):
        delete_name = name or self.name
        try:
            # close the foreign key check
            sql = "SET FOREIGN_KEY_CHECKS=0"
            self.cursor.execute(sql)

            sql = "DELETE FROM volumes WHERE display_name='%s'" % delete_name
            start_time = time.time()
            self.cursor.execute(sql)
            self.db.commit()
        except Exception as err:
            self.db.rollback()
        finally:
            end_time = time.time()
            print "Delete inserted data from volumes cost: %s seconds" % \
                  (end_time - start_time)
            self.db.close()

    def get_volume_num(self, name="test_pressure_mysql"):
        get_name = name or self.name
        sql = "select count(id) from volumes where display_name='%s'" % get_name
        try:
            start_time = time.time()
            self.cursor.execute(sql)
            result = self.cursor.fetchall()
        except Exception as err:
            self.db.rollback()
        finally:
            end_time = time.time()
            print "Get intems num of volume cost: %s seconds" % \
                  (end_time - start_time)
            print "Current items num of volumes database: %s" % result
            self.db.close()

    def get_volume(self, volume_id):
        sql = "select * from volumes where id='%s'" % volume_id
        try:
            start_time = time.time()
            self.cursor.execute(sql)
            result = self.cursor.fetchall()
        except Exception as err:
            self.db.rollback()
        finally:
            end_time = time.time()
            print "Search volumes %s cost: %s seconds" % \
                  (volume_id, end_time - start_time)
            self.db.close()

    def insert_port(self, name='test_pressure_mysql',
                      count=100000):
        try:
            start_time = time.time()
            self.name = name
            for i in range(count):
                tenant_id = uuid.uuid4()
                id = tenant_id
                name = name
                network_id = id
                mac_address = "fa:16:3e:11:fa:e5"
                admin_state_up = 1
                status = 'ACTIVE'
                device_id = tenant_id
                device_owner = "compute:dev"

                sql = "INSERT INTO ports(tenant_id, id, name, network_id, \
                       mac_address, admin_state_up, status, device_id, \
                       device_owner) VALUES ('%s', '%s', '%s', \
                       '%s', '%s', '%s', '%s', '%s', '%s')" % \
                       (tenant_id, id, name, network_id, mac_address, \
                       admin_state_up, status, device_id, device_owner)
                try:
                    self.cursor.execute(sql)
                    self.db.commit()
                except:
                    self.db.rollback()
            end_time = time.time()
            print "Insert %s item to port cost: %s seconds" % \
                  (count, end_time - start_time)
        except:
            pass
        finally:
            self.db.close()

    def delete_port(self, name='test_pressure_mysql'):
        delete_name = name or self.name
        try:
            # close the foreign key check
            sql = "SET FOREIGN_KEY_CHECKS=0"
            self.cursor.execute(sql)

            sql = "DELETE FROM ports WHERE name='%s'" % delete_name
            start_time = time.time()
            self.cursor.execute(sql)
            self.db.commit()
        except Exception as err:
            self.db.rollback()
        finally:
            end_time = time.time()
            print "Delete inserted data from ports cost: %s seconds" % \
                  (end_time - start_time)
            self.db.close()

    def get_port_num(self, name="test_pressure_mysql"):
        get_name = name or self.name
        sql = "select count(id) from ports where display_name='%s'" % get_name
        try:
            start_time = time.time()
            self.cursor.execute(sql)
            result = self.cursor.fetchall()
        except Exception as err:
            self.db.rollback()
        finally:
            end_time = time.time()
            print "Get intems num of ports cost: %s seconds" % \
                  (end_time - start_time)
            print "Current items num of ports database: %s" % result
            self.db.close()

    def get_port(self, port_id):
        sql = "select * from ports where id='%s'" % port_id
        try:
            start_time = time.time()
            self.cursor.execute(sql)
            result = self.cursor.fetchall()
        except Exception as err:
            self.db.rollback()
        finally:
            end_time = time.time()
            print "Search ports %s cost: %s seconds" % \
                  (port_id, end_time - start_time)
            self.db.close()


if __name__ == '__main__':
    db_test = Mysql()
    db_test.insert_instance()

    print "round 2"
    db_test.reconnect()
    db_test.insert_instance()

    db_test.reconnect()
    db_test.delete_instance()
