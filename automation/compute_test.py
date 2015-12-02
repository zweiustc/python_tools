#!/usr/bin/env python
import httplib2
import json
import logging
import pexpect
import random
import time

logfile='/var/log/compute_test.log'
logging.basicConfig(level=logging.DEBUG,
                    format='%(asctime)s %(filename)s[line:%(lineno)d] %(levelname)s %(message)s',
                    datefmt='%Y %b %d %H:%M:%S',
                    filename=logfile,
                    filemode='a')


DEFAULT_HTTP_URI_CHARSET = 'UTF-8'
DEFAULT_HTTP_BODY_CHARSET = 'UTF-8'
DEFAULT_HTTP_HEADER_CHARSET = 'UTF-8'
LOG_TIME_FORMAT = '%Y-%m-%d %X'

#status
SUCCESS = "success"
AVAILABLE = "available"
ERROR = "error"

#endpoint
KEYSTONE_HOST = '10.160.60.42'
KEYSTONE_PORT = '5000'
KEYSTONE_TOKEN_URL = 'http://' + KEYSTONE_HOST + ':' + KEYSTONE_PORT + '/v2.0/tokens '
NOVA_BASE_URL = 'http://10.160.60.42:8774/v2'
NEUTRON_BASE_URL = 'http://10.160.60.42:9696/v2.0'
CINDER_BASE_URL = 'http://10.160.60.42:8776/v2'

TEST_TENANT = 'admin'
TEST_user = 'admin'
TEST_passw0rd = 'openstack1'
GUESTOS_PASSWD = 'test'
TEST_KEEP_ENV = False

#external variables
IMAGEID='470112e6-845b-4eaa-aa35-6bc567c6612f'
NETID='568e1807-89c6-46e4-83a2-357ef39812e0'
TENANTID='82bdaf4930854ae7a4b494dba9563f50'
EXTERNAL_NETWORK_ID = '3de42525-8bf2-4449-8677-4de6cffe2d32'

global test_token
test_token = None
global test_tenant_id
test_tenant_id = TENANTID
global test_volume_id
test_volume_id = None
global test_server_id
test_server_id = None
global test_floating_ip_id
test_floating_ip_id = None


def _get_hostport(authority):
    i = authority.rfind(':')
    host = str(authority[0:i])
    port = int(authority[i + 1:])
    return (host, port)

def send_http_request(method, request_url, body=None, request_headers={}):

    uri = httplib2.iri2uri(request_url)
    (scheme, authority, request_uri) = httplib2.urlnorm(uri)[:3]
    address = _get_hostport(authority)
    http_client = httplib2.HTTPConnectionWithTimeout(address[0], port=address[1])
    if http_client.sock is None:
        http_client.connect()

    http_client.putrequest(method,
                           request_uri.encode(DEFAULT_HTTP_URI_CHARSET),
                           {'skip_host': 1, 'skip_accept_encoding': 1})

    for key, value in request_headers.items():
        http_client.putheader(key, value.encode(DEFAULT_HTTP_HEADER_CHARSET))
    http_client.endheaders()
    if body:
        http_client.send(body)
    return http_client.getresponse()

def _get_common_headers(request_body):
    headers = {}
    if test_token:
        headers['X-Auth-Token'] = test_token
    if request_body:
        headers['Content-Type'] = 'application/json'
        headers['Content-length'] = str(len(request_body))
    return headers


def _get_Token():
    headers = {}
    request_body = json.dumps(
                                {
                                    "auth": {
                                        "tenantName": TEST_TENANT,
                                        "passwordCredentials": {
                                            "username": TEST_user,
                                            "password": TEST_passw0rd
                                        }
                                    }
                                }
                              )
    headers['Content-Type'] = 'application/json'
    headers['Content-length'] = str(len(request_body))
    response = send_http_request("POST", KEYSTONE_TOKEN_URL, request_body, headers)
    global test_token
    test_token = json.loads(response.read())["access"]["token"]["id"]

def http_post(url, request_body):
    request_id = str(time.time())
    headers = _get_common_headers(request_body)
    logging.info(" ID:" + request_id + " POST:"+ url + " BODY:" + request_body + " HEADERS:" + str(headers))
    
    response = send_http_request('POST', url, request_body, headers)
    status = response.status
    response_body = response.read();
    logging.info(" ID:" + request_id + " STATUS:"+ str(status) + " BODY:" + response_body)
    if status == 401:
        logging.warning('the status is 401, resend the request')
        _get_Token()
        return http_post(url, request_body)
    return (status, response_body)

def http_put(url, request_body):
    request_id = str(time.time())
    headers = _get_common_headers(request_body)
    logging.info(" ID:" + request_id + " POST:"+ url + " BODY:" + request_body + " HEADERS:" + str(headers))
    
    response = send_http_request('PUT', url, request_body, headers)
    status = response.status
    response_body = response.read();
    logging.info(" ID:" + request_id + " STATUS:"+ str(status) + " BODY:" + response_body)
    if status == 401:
        logging.warning('the status is 401, resend the request')
        _get_Token()
        return http_post(url, request_body)
    return (status, response_body)

def http_get(url):
    request_id = str(time.time())
    headers = _get_common_headers(None)
    logging.info(" ID:" + request_id + " GET:"+ url + " HEADERS:" + str(headers))
    
    response = send_http_request('GET', url, None, headers)
    status = response.status
    response_body = response.read();
    logging.info(" ID:" + request_id + " STATUS:"+ str(status) + " BODY:" + response_body)
    if status == 401:
        logging.warning('the status is 401, resend the request')
        _get_Token()
        return http_get(url)
    return (status, response_body)
    
def http_delete(url):
    request_id = str(time.time())
    headers = _get_common_headers(None)
    logging.info(" ID:" + request_id + " DELETE:"+ url + " HEADERS:" + str(headers))
    
    response = send_http_request('DELETE', url, None, headers)
    status = response.status
    response_body = response.read();
    logging.info(" ID:" + request_id + " STATUS:"+ str(status) + " BODY:" + response_body)
    if status == 401:
        _get_Token()
        return http_delete(url)
    return (status, response_body)

def generate_uid(topic, size=8):
    "Create random name."
    characters = '01234567890abcdefghijklmnopqrstuvwxyz'
    choices = [random.choice(characters) for _x in xrange(size)]
    return '%s-%s' % (topic, ''.join(choices))


def create_volume(name, tenant_id=TENANTID, image_id=IMAGEID, size=20):
    url = CINDER_BASE_URL + "/%s/volumes" % tenant_id
    request_body = json.dumps(
                              {
                                "volume": {
                                    "size": size,
                                    "name": name,
                                    "imageRef": image_id,
                                    "attach_status": "detached",
                                    "metadata": {}
                                }
                              }
                              )
    status, response_body = http_post(url, request_body)
    return json.loads(response_body)["volume"]["id"]

def get_volume_status(tenant_id, volume_id):
    url = CINDER_BASE_URL + "/%s/volumes/%s" % (tenant_id, volume_id)
    status, response_body = http_get(url)
    return json.loads(response_body)["volume"]["status"]

def delete_volume(tenant_id, volume_id):
    url = CINDER_BASE_URL + "/%s/volumes/%s" % (tenant_id, volume_id)
    status, response_body = http_delete(url)
    return status

def boot_from_volume(volume_id, flavor_id, net_id, name):
    url = NOVA_BASE_URL + "/%s/os-volumes_boot" % TENANTID
    request_body = json.dumps(
                              {
                                "server":{
                                    "name": name,
                                    "imageRef":"",
                                    "block_device_mapping_v2":[
                                        {
                                            "source_type":"volume",
                                            "delete_on_termination":False,
                                            "boot_index":0,
                                            "uuid":volume_id,
                                            "destination_type":"volume"
                                        }
                                    ],
                                    "flavorRef":flavor_id,
                                    "max_count":1,
                                    "min_count":1,
                                    "networks":[{"uuid":net_id}]
                                }
                              }
                              
                            )
    status, response_body = http_post(url, request_body)
    return json.loads(response_body)["server"]["id"];

def get_server_status(tenant_id, server_id):
    url = NOVA_BASE_URL + "/%s/servers/%s" % (tenant_id, server_id)
    status, response_body = http_get(url)
    return json.loads(response_body)["server"]["status"]

def get_server_name(tenant_id, server_id):
    url = NOVA_BASE_URL + "/%s/servers/%s" % (tenant_id, server_id)
    status, response_body = http_get(url)
    return json.loads(response_body)["server"]["name"]

def get_server_host(tenant_id, server_id):
    url = NOVA_BASE_URL + "/%s/servers/%s" % (tenant_id, server_id)
    status, response_body = http_get(url)
    return json.loads(response_body)["server"]["OS-EXT-SRV-ATTR:host"]

def reboot_server(tenant_id, server_id):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (tenant_id, server_id)
    request_body = json.dumps(
                              {
                                "reboot":{
                                    "type": "SOFT"
                                }
                              }
                            )
    status, response_body = http_post(url, request_body)
    return status

def stop_server(tenant_id, server_id):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (tenant_id, server_id)
    request_body = json.dumps(
                              {
                                "os-stop":None
                              }
                            )
    status, response_body = http_post(url, request_body)
    return status

def start_server(tenant_id, server_id):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (tenant_id, server_id)
    request_body = json.dumps(
                              {
                                "os-start":None
                              }
                            )
    status, response_body = http_post(url, request_body)
    return status

def rename_server(tenant_id, server_id):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (tenant_id, server_id)
    request_body = json.dumps(
                              {
                                "server":{
                                    "name": name
                                }
                              }
                            )
    status, response_body = http_post(url, request_body)
    return status

def delete_server(tenant_id, server_id):
    url = NOVA_BASE_URL + "/%s/servers/%s" % (tenant_id, server_id)
    status, response_body = http_delete(url)
    if status != 204:
        logging.error(("Delete Server %s failed") % server_id)
    return status

def floatingip_create():
    url = NEUTRON_BASE_URL + "/floatingips"
    request_body = json.dumps(
                              {
                                "floatingip": {
                                    "floating_network_id": EXTERNAL_NETWORK_ID
                                }
                              }
                             )
    status, response_body = http_post(url, request_body)
    if status != 201:
        logging.error(("floatingip create failed"))
        raise Exception("floatingip create failed")
    response_body_ob = json.loads(response_body)
    return response_body_ob["floatingip"]["id"], response_body_ob["floatingip"]["floating_ip_address"];

def floatingip_delete(floatingip_id):
    url = NEUTRON_BASE_URL + "/floatingips/" + floatingip_id
    status, response_body = http_delete(url)
    if status != 204:
        logging.error(("floatingip delete failed"))
        raise Exception("floatingip delete failed")
    return response_body

def floating_ip_associate(tenant_id, server_id, floating_ip):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (tenant_id, server_id)
    request_body = json.dumps(
                              {
                                "addFloatingIp":{
                                    "address": floating_ip
                                }
                              }
                            )
    status, response_body = http_post(url, request_body)
    return status

def floating_ip_disassociate(tenant_id, server_id, floating_ip):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (tenant_id, server_id)
    request_body = json.dumps(
                              {
                                "removeFloatingIp":{
                                    "address": floating_ip
                                }
                              }
                            )
    status, response_body = http_post(url, request_body)
    return status

def ping_floating_ip(server_id, floating_ip):
    cmd = "ping -c 5 %s" % floating_ip
    logging.info(cmd)
    child = pexpect.spawn(cmd)
    fout = file(logfile, 'a')
    index = child.expect(['0% packet loss', 'Host Unreachable',
                         pexpect.EOF, pexpect.TIMEOUT], timeout=30)
    logging.info(child.before)
    if index == 0:
        logging.info(("server %s with floating ip %s "
                    "is reachable") % (server_id, floating_ip))
    elif index == 1:
        logging.error(("server %s with floating ip %s "
                     "is unreachable") % (server_id, floating_ip))
        raise Exception("The floating_ip is unreachable")
    else:
        logging.warning(("server %s with floating ip %s "
                        "ping timeout") % (server_id, floating_ip))

def ping_floating_ip_after_remove(server_id, floating_ip):
    cmd = "ping -c 5 %s" % floating_ip
    logging.info(cmd)
    child = pexpect.spawn(cmd)
    fout = file(logfile, 'a')
    index = child.expect(['0% packet loss', '100% packet loss',
                         pexpect.EOF, pexpect.TIMEOUT], timeout=30)
    logging.info(child.before)
    if index == 0:
        logging.error(("server %s failed to remove ip: %s ")\
                    % (server_id, floating_ip))
    elif index == 1:
        logging.info(("server %s remove ip: %s"
                     "successfully") % (server_id, floating_ip))
    else:
        logging.warning(("server %s with floating ip %s "
                        "ping timeout") % (server_id, floating_ip))

def migrate(tenant_id, server_id):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (tenant_id, server_id)
    request_body = json.dumps(
                              {
                                "migrate":None
                              }
                            )
    status, response_body = http_post(url, request_body)
    return status

def live_migrate(tenant_id, server_id, host):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (tenant_id, server_id)
    request_body = json.dumps(
                              {
                                "os-migrateLive":{
                                    "disk_over_commit": False,
                                    "block_migration": False,
                                    "host": host
                                }
                              }
                            )
    status, response_body = http_post(url, request_body)
    return status

def resize_server(tenant_id, server_id, flavor_id):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (tenant_id, server_id)
    request_body = json.dumps(
                              {
                                "resize": {
                                    "flavorRef": flavor_id
                                }
                              }
                            )
    status, response_body = http_post(url, request_body)
    return status

def resize_confirm_server(tenant_id, server_id):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (tenant_id, server_id)
    request_body = json.dumps(
                              {
                                "confirmResize": None
                              }
                            )
    status, response_body = http_post(url, request_body)
    if status != 204:
        logging.error(("Server resize confirm %s failed") % server_id)
    return status

def ssh_check_mem(floating_ip, passwd):
    cmd = "free -g | grep Mem |awk '{print $2}'"
    ssh = pexpect.spawn('ssh root@%s "%s"' % (floating_ip, cmd),timeout=120)
    logging.info('ssh root@%s "%s"' % (floating_ip, cmd))
    mem = 0
    try:
        i = ssh.expect(['password:', 'continue connecting (yes/no)?'], timeout=60)
        if i == 0 :
            ssh.sendline(passwd)
        elif i == 1:
            ssh.sendline('yes\n')
            ssh.expect('password: ')
            ssh.sendline(passwd)
        ssh.sendline(cmd)
        result = ssh.read()
        mem = result.split('\n')[1]
        return int(mem)
    except pexpect.EOF:
        logging.warning('ssh root@%s EOF' % floating_ip)
        ssh.close()
        return int(mem)
    except pexpect.TIMEOUT:
        logging.warning('ssh root@%s timeout' % floating_ip)
        ssh.close()
        return int(mem)

def ssh_check_cpu(floating_ip, passwd):
    cmd = "cat /proc/cpuinfo | grep processor | wc -l"
    ssh = pexpect.spawn('ssh root@%s "%s"' % (floating_ip, cmd),timeout=120)
    logging.info('ssh root@%s "%s"' % (floating_ip, cmd))
    cpu_num = 0
    try:
        i = ssh.expect(['password:', 'continue connecting (yes/no)?'], timeout=60)
        if i == 0 :
            ssh.sendline(passwd)
        elif i == 1:
            ssh.sendline('yes\n')
            ssh.expect('password: ')
            ssh.sendline(passwd)
        ssh.sendline(cmd)
        result = ssh.read()
        cpu_num = result.split('\n')[1]
        return int(cpu_num)
    except pexpect.EOF:
        ssh.close()
        logging.warning('ssh root@%s EOF' % floating_ip)
        return int(cpu_num)
    except pexpect.TIMEOUT:
        ssh.close()
        logging.warning('ssh root@%s timeout' % floating_ip)
        return int(cpu_num)

def test_boot_from_volume(tenant_id, image_id, flavor_id):
    #create a random name
    name = generate_uid('test_compute_zhangwei', size=8)

    #create cinder volume
    logging.info(("Begin to create cinder volume %s") % name)
    volume_id = create_volume(name, tenant_id=TENANTID,
                              image_id=IMAGEID, size=20)
    time.sleep(60)

    for index in xrange(20):
        status = get_volume_status(TENANTID, volume_id)
        if status == "available":
            break
        else:
            logging.info(("wait 15s for the volume %s to be available") % volume_id)
            time.sleep(15)

    #check the volume status
    logging.info(("Get the volume status %s") % name)
    status = get_volume_status(TENANTID, volume_id)
    if status != "available":
        logging.error(("the volume %s failed to create with image") % volume_id)

    #boot the server from volume
    logging.info(("Begin to boot vm %s from volume %s") % (name, volume_id))
    server_id = boot_from_volume(volume_id,
                                 flavor_id, NETID, name)
    time.sleep(60)
    return server_id, volume_id

def test_reboot(tenant_id, server_id):
    logging.info(("reboot the server status %s") % server_id)
    status = reboot_server(tenant_id, server_id)
    expect_state = "REBOOT"
    #check the server status
    logging.info(("Get the server status %s") % server_id)
    status = get_server_status(tenant_id, server_id)
    if status != expect_state:
        logging.error(("failed to catch the reboot state of %s") % server_id)
        raise Exception("Failed to reboot server")
    time.sleep(60)
    logging.info(("Get the server status %s after 10s") % server_id)
    status = get_server_status(tenant_id, server_id)
    if status != 'ACTIVE':
        logging.error(("Server %s failed to be active after reboot") % server_id)
        raise Exception("Failed to reboot server")
    time.sleep(60)

def test_resize(tenant_id, server_id, floating_ip):
    logging.info(("Stop the server %s before resize") % server_id)
    stop_server(tenant_id, server_id)
    time.sleep(60)
    status = get_server_status(tenant_id, server_id)
    if status != 'SHUTOFF':
        logging.error(("failed to stop %s") % server_id)
        raise Exception("Failed to stop server when resize")
    time.sleep(30)
    new_flavor_id = 3
    if status == 'SHUTOFF':
        resize_server(tenant_id, server_id, new_flavor_id)
        time.sleep(30)
    status = get_server_status(tenant_id, server_id)
    for index in xrange(6):
        status = get_server_status(tenant_id, server_id)
        if status == "VERIFY_RESIZE":
            break
        else:
            logging.info(("wait 15s for server %s to be VERIFY_RESIZE") % server_id)
            time.sleep(10)
    if status == "VERIFY_RESIZE":
        resize_confirm_server(tenant_id, server_id)
        time.sleep(10)
    else:
        logging.error(("The server %s hang in %s state") % (server_id, status))
        raise Exception("Failed to resize to VERIFY_RESIZE")
    #start the server
    logging.info(("Start the server %s") % server_id)
    start_server(tenant_id, server_id)
    time.sleep(60)
    for index in xrange(6):
        status = get_server_status(tenant_id, server_id)
        if status == "ACTIVE":
            break
        else:
            logging.info(("wait 15s for server %s to be active") % server_id)
            time.sleep(10)
    status = get_server_status(tenant_id, server_id)
    if status != 'ACTIVE':
        logging.error(("failed to start %s, stay in state: %s") % (server_id, status))
        raise Exception("Failed to start server after resize")
    time.sleep(40)
    mem_resized =  ssh_check_mem(floating_ip, GUESTOS_PASSWD)
    cpu_num_resized =  ssh_check_cpu(floating_ip, GUESTOS_PASSWD)
    if mem_resized == 1 and cpu_num_resized == 1:
        logging.error(("Resize Server %s failed: mem: %s "
                     "cpu_num: %s") % (server_id, mem_resized, cpu_num_resized))
        raise Exception("Resize failed")
    else:
        logging.info(("Resize Server %s successfully: mem: %s "
                     "cpu_num: %s") % (server_id, mem_resized, cpu_num_resized))

def test_live_migrate(tenant_id, server_id, floating_ip):
    #migrate
    logging.info(("Live Migrate the server %s") % server_id)
    old_host = get_server_host(tenant_id, server_id)
    if "66" in old_host or "67" in old_host:
        dest_host = "bjzjm01-zhengqi-cp-compute060069.bjzjm01.ksyun.com"
    else:
        dest_host = "bjzjm01-zhengqi-cp-compute060066.bjzjm01.ksyun.com"

    live_migrate(tenant_id=tenant_id, server_id=server_id, host=dest_host)
    time.sleep(60)
    for index in xrange(10):
        status = get_server_status(tenant_id, server_id)
        if status == "VERIFY_RESIZE" or status == "ACTIVE":
            break
        else:
            logging.warning(("wait 15s for server %s to complish live migrate") % server_id)
            time.sleep(15)
    server_status = get_server_status(tenant_id, server_id)
    if server_status =="VERIFY_RESIZE":
        logging.warning(("The server %s turn to be VERIFY_RESIZE after live migrate") % server_id)
        resize_confirm_server(tenant_id, server_id)
        time.sleep(20)
    #check the resize confirm
    server_status = get_server_status(tenant_id, server_id)
    if server_status != 'ACTIVE':
        logging.error(("server %s stay in %s after "
                     " live migrate") % (server_id, server_status))
        raise Exception("Live Migrate process hasn't been detected")
    new_host = get_server_host(tenant_id, server_id)
    if old_host == new_host:
        logging.error(("Server %s failed to live migrate") % server_id)
        raise Exception("Server live migrate failed")
    else:
        logging.info(("Server %s live migrate from %s to %s "
                    "successfully") % (server_id, old_host, new_host))
    time.sleep(30)
    logging.info(("Ping the server %s after Migrate") % server_id)
    ping_floating_ip(server_id, floating_ip)

def test_migrate(tenant_id, server_id, floating_ip):
    #migrate
    logging.info(("Migrate the server %s") % server_id)
    old_host = get_server_host(tenant_id, server_id)
    migrate(tenant_id=tenant_id, server_id=server_id)
    time.sleep(60)
    server_status = get_server_status(tenant_id, server_id)
    for index in xrange(10):
        status = get_server_status(tenant_id, server_id)
        if status == "VERIFY_RESIZE" or status == "ACTIVE":
            break
        else:
            logging.warning(("wait 15s for server %s to complish cold migrate") % server_id)
            time.sleep(15)
    if server_status =="VERIFY_RESIZE":
        logging.warning(("The server %s turn to be VERIFY_RESIZE after migrate") % server_id)
        resize_confirm_server(tenant_id, server_id)
        time.sleep(20)
    #check the resize confirm
    server_status = get_server_status(tenant_id, server_id)
    if server_status != 'ACTIVE':
        logging.error(("server %s stay in %s after "
                     "migrate") % (server_id, server_status))
        raise Exception("Migrate process hasn't been detected")
    new_host = get_server_host(tenant_id, server_id)
    if old_host == new_host:
        logging.error(("Server %s failed to migrate") % server_id)
        raise Exception("Server migrate failed")
    else:
        logging.info(("Server %s migrate from %s to %s "
                    "successfully") % (server_id, old_host, new_host))
    time.sleep(30)
    logging.info(("Ping the server %s after Migrate") % server_id)
    ping_floating_ip(server_id, floating_ip)

def clean_env(floating_ip, tenant_id, server_id, volume_id, floating_ip_id):
    if volume_id is None:
        logging.error(("clean env failed for values are None"))
        return
    try:
        #delete the floating ip
        logging.info(("Begin to delete the floating ip %s") % floating_ip)
        if floating_ip != "10.160.60.193":
            floatingip_delete(floating_ip_id)
    except Exception as e:
        logging.error(("clean env failed for %s") % e)

    try:
        #delete the Server
        logging.info(("Delete the server %s") % server_id)
        delete_server(tenant_id, server_id)
        time.sleep(10)
    except Exception as e:
        logging.error(("clean env failed for %s") % e)

    try:
        for index in xrange(6):
            status = get_volume_status(tenant_id, volume_id)
            if status == "available":
                break
            else:
                logging.info(("wait 15s for the volume %s to be available") % volume_id)
                time.sleep(10)
        logging.info(("Delete the volume %s") % volume_id)

        #delete the volume
        delete_volume(tenant_id, volume_id)
    except Exception as e:
        logging.error(("clean env failed for %s") % e)


def testcases_one_round():

    ###################################for debug####
    #name = "test_compute-g00sirt9"
    #server_id = "205156cb-db74-4182-a205-756ba5b19a2e"
    #volume_id = "28a05962-ebf0-4566-bc29-7aafcb69f0f8"
    #flavor_id = 2

    #boot a server from volume
    try:
        server_id, volume_id = test_boot_from_volume(
                                tenant_id=TENANTID,
                                image_id=IMAGEID,
                                flavor_id=2)
        global test_server_id
        test_server_id = server_id
        global test_volume_id
        test_volume_id = volume_id
        logging.info(("test_server_id: %s test_volume_id: "
                    "%s") % (test_server_id, test_volume_id))
    except Exception as e:
        logging.error(("boot server from volume failed for %s") % e)
        raise Exception("boot server from volume failed")

    #check the server status
    logging.info(("Get the server status %s") % server_id)
    status = get_server_status(TENANTID, server_id)

    #reboot the server
    test_reboot(tenant_id=TENANTID, server_id=server_id)

    #stop the server
    logging.info(("Stop the server %s") % server_id)
    stop_server(TENANTID, server_id)
    time.sleep(60)
    status = get_server_status(TENANTID, server_id)
    if status != 'SHUTOFF':
        logging.error(("failed to stop %s") % server_id)
        raise Exception("Stop the server failed")
    time.sleep(60)

    #start the server
    logging.info(("Start the server %s") % server_id)
    start_server(TENANTID, server_id)
    time.sleep(60)
    status = get_server_status(TENANTID, server_id)
    if status != 'ACTIVE':
        logging.error(("failed to start %s") % server_id)
    time.sleep(30)

    #create the floating ip
    time.sleep(10)
    logging.info(("Begin to create floating ip"))
    try:
        floating_ip_id, floating_ip = floatingip_create()
        global test_floating_ip_id
        test_floating_ip_id = floating_ip_id
        logging.info(("test_floating_ip_id: %s") % test_floating_ip_id)
    except Exception as err:
        logging.error(("Create floatingip failed for %s") % err)
        floating_ip_id = "889e0ef2-0579-4b45-b9f2-5d245fa38cb5"
        floating_ip = "10.160.60.193"

    #add the floating ip
    time.sleep(10)
    logging.info(("Associate ip: %s to the server %s") % (floating_ip, server_id))
    floating_ip_associate(TENANTID, server_id, floating_ip)
    time.sleep(30)
    ping_floating_ip(server_id, floating_ip)

    #resize the server
    test_resize(tenant_id=TENANTID, server_id=server_id, floating_ip=floating_ip)

    #live migrate the server
    test_live_migrate(tenant_id=TENANTID, server_id=server_id, floating_ip=floating_ip)

    #migrate the server
    test_migrate(tenant_id=TENANTID, server_id=server_id, floating_ip=floating_ip)

    #remove the floating ip
    time.sleep(10)
    logging.info(("Disassociate ip: %s to the server %s") % (floating_ip, server_id))
    floating_ip_disassociate(TENANTID, server_id, floating_ip)
    time.sleep(60)
    ping_floating_ip_after_remove(server_id, floating_ip)
    tenant_id = TENANTID
    return tenant_id, server_id, volume_id, floating_ip, floating_ip_id

if __name__ == "__main__":
    for index in xrange(200):
        try:
            logging.info(("---------    Round %s Begins         --------") % index)
            tenant_id, server_id, volume_id, floating_ip, floating_ip_id = [None, None, None, None, None]
            tenant_id, server_id, volume_id, floating_ip, floating_ip_id = testcases_one_round()
            clean_env(floating_ip=floating_ip,
                      tenant_id=test_tenant_id,
                      server_id=test_server_id,
                      volume_id=test_volume_id,
                      floating_ip_id=test_floating_ip_id)
        except Exception:
            logging.warning(("-----------exception detected in Round %s------------") % index)
            if not TEST_KEEP_ENV:
                clean_env(floating_ip=floating_ip,
                          tenant_id=test_tenant_id,
                          server_id=test_server_id,
                          volume_id=test_volume_id,
                          floating_ip_id=test_floating_ip_id)
        finally:
            logging.info("----------------------------------------------")
            logging.info("---------                             --------")
            logging.info(("---------    Round %s Arrived End    --------") % index)
            logging.info("---------                             --------")
            logging.info("---------                             --------")
            logging.info("----------------------------------------------")
