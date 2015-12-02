#!/usr/bin/env python

import httplib2
import json
import time
import traceback
import commands
import pexpect
TEST_DEBUG = True
TEST_LOG_MAIN = True
TEST_RUN_COUNT = 1
TEST_KEEP_ENV = True
TEST_BOOT_VOLUME = True
TEST_VM_COUNT = 6
TEST_VM_ROUND_COUNT = 2

DEFAULT_HTTP_URI_CHARSET = 'UTF-8'
DEFAULT_HTTP_BODY_CHARSET = 'UTF-8'
DEFAULT_HTTP_HEADER_CHARSET = 'UTF-8'
LOG_TIME_FORMAT = '%Y-%m-%d %X'

KEYSTONE_HOST = '10.160.60.42'
KEYSTONE_TOKEN_URL = 'http://' + KEYSTONE_HOST + ':5000/v2.0/tokens '
NEUTRON_BASE_URL = 'http://' + KEYSTONE_HOST + ':9696/v2.0'
CINDER_BASE_URL = 'http://' + KEYSTONE_HOST + ':8776/v2'
NOVA_BASE_URL = 'http://' + KEYSTONE_HOST + ':8774/v2'
TEST_TENANT = 'longrun'
TEST_TENANT_ID = 'b3acb7d2d5204786ab65d6acf9518f47'
TEST_user = 'admin'
TEST_passw0rd = 'admin@kingsoft'
TEST_GUESTOS_PASSWD = 'kingsoft123!@#'

TEST_SUBNET_CIDR = '192.168.0.0/24'
TEST_EXTERNAL_NETWORK_ID = '947f7a44-6685-403c-b114-66ef3ded1661'
TEST_IMAGE_ID = '59909769-0a17-4d34-94bc-703e24ffd775'
TEST_SNAPSHOT_ID = '0770c568-bbfd-474f-8a9c-f254d4ce9164'
TEST_FLAVOR_ID = '3'
TEST_NAME_FIX = str(time.time())

global test_token
test_token = None
global log_buffer
log_buffer = []


def _get_hostport(authority):
    i = authority.rfind(':')
    host = str(authority[0:i])
    port = int(authority[i + 1:])
    return (host, port)


def send_http_request(method, request_url, body=None, request_headers={}):

    uri = httplib2.iri2uri(request_url)
    (scheme, authority, request_uri) = httplib2.urlnorm(uri)[:3]
    address = _get_hostport(authority)
    http_client = httplib2.HTTPConnectionWithTimeout(address[0],
                                                     port=address[1])
    if http_client.sock is None:
        http_client.connect()

    http_client.putrequest(method,
                           request_uri.encode(DEFAULT_HTTP_URI_CHARSET))

    for key, value in request_headers.items():
        http_client.putheader(key, value.encode(DEFAULT_HTTP_HEADER_CHARSET))
    http_client.endheaders()
    if body:
        http_client.send(body)
    return http_client.getresponse()


def log(message):
    if TEST_DEBUG:
        print message
    else:
        log_buffer.append(message)


def log_main(message):
        print message
        log_buffer.append(message)


def print_log():
    if not TEST_DEBUG:
        for message in log_buffer:
            if not TEST_LOG_MAIN:
                print message
        drop_log()


def drop_log():
    if not TEST_DEBUG:
        global log_buffer
        log_buffer = []


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
    response_body = response.read()
    log("request token:" + response_body)
    global test_token
    test_token = json.loads(response_body)["access"]["token"]["id"]


def http_post(url, request_body):
    request_id = str(time.time())
    time_point = time.strftime(LOG_TIME_FORMAT, time.localtime())
    headers = _get_common_headers(request_body)
    log(time_point + " ID:" + request_id + " POST:" + url + " BODY:" + request_body + " HEADERS:" + str(headers))

    response = send_http_request('POST', url, request_body, headers)
    status = response.status
    response_body = response.read()
    time_point = time.strftime(LOG_TIME_FORMAT, time.localtime())
    log(time_point + " ID:" + request_id + " STATUS:" + str(status) + " BODY:" + response_body)
    if status == 401:
        _get_Token()
        return http_post(url, request_body)
    return (status, response_body)


def http_put(url, request_body):
    request_id = str(time.time())
    time_point = time.strftime(LOG_TIME_FORMAT, time.localtime())
    headers = _get_common_headers(request_body)
    log(time_point + " ID:" + request_id + " PUT:" + url + " BODY:" + request_body + " HEADERS:" + str(headers))

    response = send_http_request('PUT', url, request_body, headers)
    status = response.status
    response_body = response.read()
    time_point = time.strftime(LOG_TIME_FORMAT, time.localtime())
    log(time_point + " ID:" + request_id + " STATUS:" + str(status) + " BODY:" + response_body)
    if status == 401:
        _get_Token()
        return http_put(url, request_body)
    return (status, response_body)


def http_get(url):
    request_id = str(time.time())
    time_point = time.strftime(LOG_TIME_FORMAT, time.localtime())
    headers = _get_common_headers(None)
    log(time_point + " ID:" + request_id + " GET:" + url + " HEADERS:" + str(headers))

    response = send_http_request('GET', url, None, headers)
    status = response.status
    response_body = response.read()
    time_point = time.strftime(LOG_TIME_FORMAT, time.localtime())
    log(time_point + " ID:" + request_id + " STATUS:" + str(status) + " BODY:" + response_body)
    if status == 401:
        _get_Token()
        return http_get(url)
    return (status, response_body)


def http_delete(url):
    request_id = str(time.time())
    time_point = time.strftime(LOG_TIME_FORMAT, time.localtime())
    headers = _get_common_headers(None)
    log(time_point + " ID:" + request_id + " DELETE:" + url + " HEADERS:" + str(headers))

    response = send_http_request('DELETE', url, None, headers)
    status = response.status
    response_body = response.read()
    time_point = time.strftime(LOG_TIME_FORMAT, time.localtime())
    log(time_point + " ID:" + request_id + " STATUS:" + str(status) + " BODY:" + response_body)
    if status == 401:
        _get_Token()
        return http_delete(url)
    return (status, response_body)


def monitor_user_login():
    request_id = str(time.time())
    time_point = time.strftime(LOG_TIME_FORMAT, time.localtime())
    data = json.dumps(
                      {
                        "jsonrpc": "2.0",
                        "method": "user.login",
                        "params": {
                            "user": "Admin",
                            "password": "zabbix"
                         },
                        "id": 0
                      })
    headers = _get_common_headers(data)
    log(time_point + " ID:" + request_id + " POST:http://10.160.60.66:80/zabbix/api_jsonrpc.php HEADERS:" + str(headers) + " BODY:" + data)
    response = send_http_request('GET', 'http://10.160.60.66:80/zabbix/api_jsonrpc.php', data, headers)
    status = response.status
    response_body = response.read()
    time_point = time.strftime(LOG_TIME_FORMAT, time.localtime())
    log(time_point + " ID:" + request_id + " STATUS:" + str(status) + " BODY:" + response_body)
    response_body_ob = json.loads(response_body)
    authID = response_body_ob['result']
    return authID


def monitor_host_create(auth_id, instance_id, instance_ip):
    hostip = instance_id
    g_list=[{"groupid": "2"}]
    t_list=[{"templateid": "10002"}]
    if instance_id:
        request_id = str(time.time())
        time_point = time.strftime(LOG_TIME_FORMAT, time.localtime())
        data = json.dumps(
                 {
                     "jsonrpc": "2.0",
                     "method": "host.create",
                     "params": {
                         "host": instance_id,
                         "interfaces": [
                             {
                                 "type": 1,
                                 "main": 1,
                                 "useip": 1,
                                 "ip": instance_ip,
                                 "dns": "",
                                 "port": "10050"
                             }
                         ],
                         "groups": g_list,
                         "templates": t_list,
                     },
                     "auth": auth_id,
                     "id": 1,
                     })
        headers = _get_common_headers(data)
        log(time_point + " ID:" + request_id + " POST:http://10.160.60.66:80/zabbix/api_jsonrpc.php HEADERS:" + str(headers) + " BODY:" + data)
        response = send_http_request('GET', 'http://10.160.60.66:80/zabbix/api_jsonrpc.php', data, headers)
        status = response.status
        response_body = response.read()
        print response_body
        time_point = time.strftime(LOG_TIME_FORMAT, time.localtime())
        log(time_point + " ID:" + request_id + " STATUS:" + str(status) + " BODY:" + response_body)
        response_body_ob = json.loads(response_body)
        return response_body_ob["result"]["hostids"]


def network_create():
    url = NEUTRON_BASE_URL + "/networks"
    request_body = json.dumps(
                              {
                                "network": {
                                    "provider:network_type": "vlan",
                                    "name": "auto_testNetwork" + TEST_NAME_FIX,
                                    "provider:physical_network": "default",
                                    "admin_state_up": True
                                    }
                                }
                              )
    status, response_body = http_post(url, request_body)
    if status != 201:
        raise Exception("network create failed")
    response_body_ob = json.loads(response_body)
    return response_body_ob["network"]["id"], response_body_ob["network"]["name"]


def network_find(network_id):
    url = NEUTRON_BASE_URL + "/networks/" + network_id
    status, response_body = http_get(url)
    return json.loads(response_body)


def network_delete(network_id):
    url = NEUTRON_BASE_URL + "/networks/" + network_id
    status, response_body = http_delete(url)
    if status != 204:
        raise Exception("network delete failed")
    return response_body


def subnet_create(network_id, cidr=TEST_SUBNET_CIDR):
    url = NEUTRON_BASE_URL + "/subnets"
    request_body = json.dumps(
                              {
                               "subnet": {
                                    "name": "auto_testSubnet" + TEST_NAME_FIX,
                                    "network_id": network_id,
                                    "ip_version": 4,
                                    "cidr": cidr
                                    }
                               }
                              )
    status, response_body = http_post(url, request_body)
    if status != 201:
        raise Exception("subnet create failed")
    return json.loads(response_body)["subnet"]["id"]


def subnet_find(subnet_id):
    url = NEUTRON_BASE_URL + "/subnets/" + subnet_id
    status, response_body = http_get(url)
    return json.loads(response_body)


def subnet_delete(subnet_id):
    url = NEUTRON_BASE_URL + "/subnets/" + subnet_id
    status, response_body = http_delete(url)
    if status != 204:
        raise Exception("subnet delete failed")
    return response_body


def router_create():
    url = NEUTRON_BASE_URL + "/routers"
    request_body = json.dumps(
                              {
                               "router": {
                                    "name": "auto_testRouter" + TEST_NAME_FIX,
                                    "admin_state_up": True
                                    }
                               }
                              )
    status, response_body = http_post(url, request_body)
    if status != 201:
        raise Exception("router create failed")
    return json.loads(response_body)["router"]["id"]


def router_find(router_id):
    url = NEUTRON_BASE_URL + "/routers/" + router_id
    status, response_body = http_get(url)
    return json.loads(response_body)


def router_set_external_gw(router_id):
    url = NEUTRON_BASE_URL + "/routers/" + router_id
    request_body = json.dumps(
                              {
                               "router": {
                                    "external_gateway_info": {
                                        "network_id": TEST_EXTERNAL_NETWORK_ID
                                        }
                                    }
                               }
                              )
    status, response_body = http_put(url, request_body)
    if status != 200:
        raise Exception("router set external gw failed")
    return response_body


def router_add_internal_gw(router_id, subnet_id):
    url = NEUTRON_BASE_URL + "/routers/" + router_id + "/add_router_interface"
    request_body = json.dumps(
                              {
                                "subnet_id": subnet_id
                                }
                              )
    status, response_body = http_put(url, request_body)
    if status != 200:
        raise Exception("router add internal gw failed")
    return response_body


def router_remove_internal_gw(router_id, subnet_id):
    url = NEUTRON_BASE_URL + "/routers/" + router_id + "/remove_router_interface"
    request_body = json.dumps(
                              {
                                "subnet_id": subnet_id
                                }
                              )
    status, response_body = http_put(url, request_body)
    if status != 200:
        raise Exception("router remove internal gw failed")
    return response_body


def router_delete(router_id):
    url = NEUTRON_BASE_URL + "/routers/" + router_id
    status, response_body = http_delete(url)
    if status != 204:
        raise Exception("router delete failed")
    return response_body


def instance_create_by_image(network_id, securitygroup_id):
    url = NOVA_BASE_URL + "/" + TEST_TENANT_ID + "/servers"
    time_point = time.strftime(LOG_TIME_FORMAT, time.localtime())
    request_body = json.dumps(
                              {
                               "server": {
                                    "name": "auto_testInstance" + time_point,
                                    "imageRef": TEST_IMAGE_ID,
                                    "flavorRef": TEST_FLAVOR_ID,
                                    "max_count": 1,
                                    "min_count": 1,
                                    "networks": [
                                         {"uuid": network_id}
                                    ],
                                    "security_groups": [
                                        {"name": securitygroup_id}
                                    ]
                                    }
                               }
                              )
    status, response_body = http_post(url, request_body)
    if status != 202:
        raise Exception("instance create failed")
    return json.loads(response_body)["server"]["id"]


def instance_create_by_volume(network_id, securitygroup_id, volume_id):
    url = NOVA_BASE_URL + "/" + TEST_TENANT_ID + "/os-volumes_boot"
    request_body = json.dumps(
                              {
                                "server": {
                                    "name": "auto_testInstance" + TEST_NAME_FIX,
                                    "imageRef": "",
                                    "block_device_mapping_v2": [
                                        {
                                            "source_type": "volume",
                                            "delete_on_termination": False,
                                            "boot_index": 0,
                                            "uuid": volume_id,
                                            "destination_type": "volume"
                                        }
                                    ],
                                    "flavorRef": TEST_FLAVOR_ID,
                                    "max_count": 1,
                                    "min_count": 1,
                                    "networks": [{"uuid": network_id}],
                                    "security_groups": [
                                        {"name": securitygroup_id}
                                    ]
                                }
                              }
                            )
    status, response_body = http_post(url, request_body)
    return json.loads(response_body)["server"]["id"]


def instance_find(instance_id):
    url = NOVA_BASE_URL + "/" + TEST_TENANT_ID + "/servers/" + instance_id
    status, response_body = http_get(url)
    return json.loads(response_body)


def instance_stop(instance_id):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (TEST_TENANT_ID, instance_id)
    request_body = json.dumps(
                              {
                                "os-stop":None
                              }
                            )
    status, response_body = http_post(url, request_body)
    if status != 202:
        raise Exception("instance stop failed")
    return status


def instance_start(instance_id):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (TEST_TENANT_ID, instance_id)
    request_body = json.dumps(
                              {
                                "os-start":None
                              }
                            )
    status, response_body = http_post(url, request_body)
    if status != 202:
        raise Exception("instance stop failed")
    return status


def instance_resize(instance_id, flavor_id):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (TEST_TENANT_ID, instance_id)
    request_body = json.dumps(
                              {
                                "resize": {
                                    "flavorRef": flavor_id
                                }
                              }
                            )
    status, response_body = http_post(url, request_body)
    if status != 202:
        raise Exception("instance resize failed")
    return status


def instance_resize_confirm(instance_id):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (TEST_TENANT_ID, instance_id)
    request_body = json.dumps(
                              {
                                "confirmResize": None
                              }
                            )
    status, response_body = http_post(url, request_body)
    if status != 204:
        raise Exception(("Server resize confirm %s failed") % instance_id)
    return status


def instance_live_migrate(instance_id):
    url = NOVA_BASE_URL + "/%s/servers/%s/action" % (TEST_TENANT_ID, instance_id)
    request_body = json.dumps(
                              {
                                "os-migrateLive":{
                                    "disk_over_commit": False,
                                    "block_migration": False,
                                    "host":None
                                }
                              }
                            )
    status, response_body = http_post(url, request_body)
    return status


def instance_find_fix_ip(instance_id, network_name):
    response_body = instance_find(instance_id)
    for value in response_body["server"]["addresses"][network_name]:
        if value["OS-EXT-IPS:type"] == "fixed":
            return value["addr"], value["OS-EXT-IPS-MAC:mac_addr"]
    return None, None


def instance_find_host(instance_id):
    url = NOVA_BASE_URL + "/%s/servers/%s" % (TEST_TENANT_ID, instance_id)
    status, response_body = http_get(url)
    return json.loads(response_body)["server"]["OS-EXT-SRV-ATTR:host"]


def instance_volume_attach(instance_id, volume_id):
    url = NOVA_BASE_URL + "/%s/servers/%s/os-volume_attachments" % (TEST_TENANT_ID, instance_id)
    request_body = json.dumps(
                              {
                                "volumeAttachment":{
                                    "volumeId": volume_id
                                }
                              }
                            )
    status, response_body = http_post(url, request_body)
    if status != 200:
        raise Exception("volume attach failed")
    return status


def instance_delete(instance_id):
    url = NOVA_BASE_URL + "/" + TEST_TENANT_ID + "/servers/" + instance_id
    status, response_body = http_delete(url)
    if status != 204:
        raise Exception("instance delete failed")
    return response_body


def instance_wait(instance_id, instance_status):
    url = NOVA_BASE_URL + "/" + TEST_TENANT_ID + "/servers/" + instance_id
    count = 0
    while count < 50:
        log("waiting for instance turn to " + str(instance_status))
        status, response_body = http_get(url)
        if status == 404:
            if instance_status:
                raise Exception("instance did not found")
            else:
                return
        response_body_ob = json.loads(response_body)
        if response_body_ob["server"]["status"] == "ERROR":
            raise Exception("instance status is ERROR")
        if response_body_ob["server"]["status"] == instance_status:
            return
        time.sleep(5)
        count = count + 1
    if count >= 50:
        raise Exception("instance wait time out!")
    log_main("wait count:" + str(count))


def port_find_by_mac(instance_mac):
    url = NEUTRON_BASE_URL + "/ports?mac_address=" + instance_mac
    status, response_body = http_get(url)
    for value in json.loads(response_body)["ports"]:
        if value["mac_address"] == instance_mac:
            return value["id"]
    return None


def port_find_router_external_port(router_id):
    url = NEUTRON_BASE_URL + "/ports?device_id=" + router_id
    status, response_body = http_get(url)
    for value in json.loads(response_body)["ports"]:
        if value["network_id"] == TEST_EXTERNAL_NETWORK_ID:
            return value["id"]
    return None


def floatingip_create(port_id):
    url = NEUTRON_BASE_URL + "/floatingips"
    request_body = json.dumps(
                              {
                               "floatingip": {
                                    "floating_network_id": TEST_EXTERNAL_NETWORK_ID,
                                    "port_id": port_id
                                    }
                               }
                              )
    status, response_body = http_post(url, request_body)
    if status != 201:
        raise Exception("floatingip create failed")
    response_body_ob = json.loads(response_body)
    return response_body_ob["floatingip"]["id"], response_body_ob["floatingip"]["floating_ip_address"]


def floatingip_delete(floatingip_id):
    url = NEUTRON_BASE_URL + "/floatingips/" + floatingip_id
    status, response_body = http_delete(url)
    if status != 204:
        raise Exception("floatingip delete failed")
    return response_body


def securitygroup_create():
    url = NEUTRON_BASE_URL + "/security-groups"
    request_body = json.dumps(
                              {"security_group": {"name": "auto_testSecurityGroup" + TEST_NAME_FIX}}
                              )
    status, response_body = http_post(url, request_body)
    if status != 201:
        raise Exception("securitygroup create failed")
    response_body_ob = json.loads(response_body)
    return response_body_ob["security_group"]["id"]


def securitygroup_delete(securitygroup_id):
    url = NEUTRON_BASE_URL + "/security-groups/" + securitygroup_id
    status, response_body = http_delete(url)
    if status != 204:
        raise Exception("securitygroup delete failed")
    return response_body


def securitygroup_rule_create(security_group_id):
    url = NEUTRON_BASE_URL + "/security-group-rules"
    request_body = json.dumps(
                              {
                               "security_group_rule": {
                                    "ethertype": "IPv4",
                                    "direction": "ingress",
                                    "remote_ip_prefix": "0.0.0.0/0",
                                    "protocol": "ICMP",
                                    "security_group_id": security_group_id
                                    }
                               }
                              )
    status, response_body = http_post(url, request_body)
    if status != 201:
        raise Exception("securitygroup rule create failed")
    response_body_ob = json.loads(response_body)
    return response_body_ob["security_group_rule"]["id"]


def securitygroup_rule_create_tcp(security_group_id):
    url = NEUTRON_BASE_URL + "/security-group-rules"
    request_body = json.dumps(
                              {
                               "security_group_rule": {
                                    "ethertype": "IPv4",
                                    "direction": "ingress",
                                    "remote_ip_prefix": "0.0.0.0/0",
                                    "protocol": "TCP",
                                    "security_group_id": security_group_id
                                    }
                               }
                              )
    status, response_body = http_post(url, request_body)
    if status != 201:
        raise Exception("securitygroup rule create failed")
    response_body_ob = json.loads(response_body)
    return response_body_ob["security_group_rule"]["id"]


def securitygroup_rule_delete(securitygroup_rule_id):
    url = NEUTRON_BASE_URL + "/security-group-rules/" + securitygroup_rule_id
    status, response_body = http_delete(url)
    if status != 204:
        raise Exception("securitygroup rule delete failed")
    return response_body


def firewall_rule_create(pingable=True):
    url = NEUTRON_BASE_URL + "/fw/firewall_rules"
    action = "allow"
    if not pingable:
        action = "deny"
    request_body = json.dumps(
                              {
                               "firewall_rule": {
                                    "protocol": "icmp",
                                    "name": "auto_testFWrule" + TEST_NAME_FIX,
                                    "enabled": "True",
                                    "source_ip_address": "0.0.0.0/0",
                                    "destination_ip_address": "0.0.0.0/0",
                                    "action": action
                                    }
                               }
                              )
    status, response_body = http_post(url, request_body)
    if status != 201:
        raise Exception("firewall_rule create failed")
    response_body_ob = json.loads(response_body)
    return response_body_ob["firewall_rule"]["id"]


def firewall_rule_create_tcp(enable=True):
    url = NEUTRON_BASE_URL + "/fw/firewall_rules"
    action = "allow"
    if not enable:
        action = "deny"
    request_body = json.dumps(
                              {
                               "firewall_rule": {
                                    "protocol": "tcp",
                                    "name": "auto_testFWrule" + TEST_NAME_FIX,
                                    "enabled": "True",
                                    "source_ip_address": "0.0.0.0/0",
                                    "destination_ip_address": "0.0.0.0/0",
                                    "action": action
                                    }
                               }
                              )
    status, response_body = http_post(url, request_body)
    if status != 201:
        raise Exception("firewall_rule create failed")
    response_body_ob = json.loads(response_body)
    return response_body_ob["firewall_rule"]["id"]


def firewall_rule_delete(firewall_rule_id):
    url = NEUTRON_BASE_URL + "/fw/firewall_rules/" + firewall_rule_id
    status, response_body = http_delete(url)
    if status != 204:
        raise Exception("firewall_rule delete failed")
    return response_body


def firewall_policy_create():
    url = NEUTRON_BASE_URL + "/fw/firewall_policies"
    request_body = json.dumps(
                              {"firewall_policy": {"name": "auto_testFWPolicy" + TEST_NAME_FIX}}
                              )
    status, response_body = http_post(url, request_body)
    if status != 201:
        raise Exception("firewall_policy create failed")
    response_body_ob = json.loads(response_body)
    return response_body_ob["firewall_policy"]["id"]


def firewall_policy_delete(firewall_policy_id):
    url = NEUTRON_BASE_URL + "/fw/firewall_policies/" + firewall_policy_id
    status, response_body = http_delete(url)
    if status != 204:
        raise Exception("firewall_policy delete failed")
    return response_body


def firewall_polcy_insert_rule(firewall_policy_id, firewall_rule_id):
    url = NEUTRON_BASE_URL + "/fw/firewall_policies/" + firewall_policy_id + "/insert_rule"
    request_body = json.dumps(
                              {
                               "insert_after": "",
                               "firewall_rule_id": firewall_rule_id,
                               "insert_before": ""
                               }
                              )
    status, response_body = http_put(url, request_body)
    if status != 200:
        raise Exception("firewall_polcy_insert_rule failed")
    return response_body


def firewall_polcy_remove_rule(firewall_policy_id, firewall_rule_id):
    url = NEUTRON_BASE_URL + "/fw/firewall_policies/" + firewall_policy_id + "/remove_rule"
    request_body = json.dumps(
                              {"firewall_rule_id": firewall_rule_id}
                              )
    status, response_body = http_put(url, request_body)
    if status != 200:
        raise Exception("firewall_polcy_remove_rule failed")
    return response_body


def firewall_create(firewall_policy_id):
    url = NEUTRON_BASE_URL + "/fw/firewalls"
    request_body = json.dumps(
                              {
                               "firewall": {
                                    "firewall_policy_id": firewall_policy_id,
                                    "admin_state_up": True,
                                    "router_ids": [],
                                    "name": "test_autoFW" + TEST_NAME_FIX
                                    }
                               }
                              )
    status, response_body = http_post(url, request_body)
    if status != 201:
        raise Exception("firewall create failed")
    response_body_ob = json.loads(response_body)
    return response_body_ob["firewall"]["id"]


def firewall_delete(firewall_id):
    url = NEUTRON_BASE_URL + "/fw/firewalls/" + firewall_id
    status, response_body = http_delete(url)
    if status != 204:
        raise Exception("firewall delete failed")
    return response_body


def firewall_find(firewall_id):
    url = NEUTRON_BASE_URL + "/fw/firewalls/" + firewall_id
    status, response_body = http_get(url)
    return json.loads(response_body)


def firewall_wait(firewall_id, firewall_status):
    url = NEUTRON_BASE_URL + "/fw/firewalls/" + firewall_id
    total = 100
    count = 0
    while count < total:
        log("waiting for firewall turn to " + str(firewall_status))
        status, response_body = http_get(url)
        if status == 404:
            if firewall_status:
                raise Exception("firewall did not found")
            else:
                return
        response_body_ob = json.loads(response_body)
        if response_body_ob["firewall"]["status"] == firewall_status:
            return
        count = count+1
        time.sleep(2)
    raise Exception("timeout waiting for firewall turn to " + str(firewall_status))


def firewall_change_router(firewall_id, router_id):
    url = NEUTRON_BASE_URL + "/fw/firewalls/" + firewall_id
    request_body = json.dumps(
                              {
                               "firewall": {
                                    "router_ids": [
                                        router_id
                                        ]
                                    }
                               }
                              )
    status, response_body = http_put(url, request_body)
    if status != 200:
        raise Exception("firewall_change_router failed")
    return response_body


def firewall_change_policy(firewall_id, policy_id):
    url = NEUTRON_BASE_URL + "/fw/firewalls/" + firewall_id
    request_body = json.dumps(
                              {
                               "firewall": {
                                    "firewall_policy_id": policy_id
                                    }
                               }
                              )
    status, response_body = http_put(url, request_body)
    if status != 200:
        raise Exception("firewall_change_router failed")
    return response_body


def firewall_remove_all_router(firewall_id):
    url = NEUTRON_BASE_URL + "/fw/firewalls/" + firewall_id
    request_body = json.dumps(
                              {
                               "firewall": {
                                    "router_ids": []
                                    }
                               }
                              )
    status, response_body = http_put(url, request_body)
    if status != 200:

        raise Exception("firewall_remove_all_router failed")
    return response_body


def volume_create(size=20):
    url = CINDER_BASE_URL + "/%s/volumes" % TEST_TENANT_ID
    request_body = json.dumps(
                              {
                                "volume": {
                                    "size": size,
                                    "name": "auto_testVolume" + TEST_NAME_FIX
                                }
                              }
                              )
    status, response_body = http_post(url, request_body)
    return json.loads(response_body)["volume"]["id"]


def volume_wait(volume_id, volume_status):
    url = CINDER_BASE_URL + "/%s/volumes/%s" % (TEST_TENANT_ID, volume_id)
    status, response_body = http_get(url)

    while True:
        log("waiting for volume turn to " + str(volume_status))
        status, response_body = http_get(url)
        if status == 404:
            if instance_status:
                raise Exception("volume did not found")
            else:
                return
        response_body_ob = json.loads(response_body)
        if response_body_ob["volume"]["status"] == "ERROR":
            raise Exception("volume status is ERROR")
        if response_body_ob["volume"]["status"] == volume_status:
            return
        time.sleep(5)


def volume_delete(volume_id):
    url = CINDER_BASE_URL + "/%s/volumes/%s" % (TEST_TENANT_ID, volume_id)
    status, response_body = http_delete(url)
    if status != 202:
        raise Exception("volume delete failed")
    return response_body


def clear_env(env):
    log("=======================clear env start=================")
    log(str(env))
    network_id = env.get("network_id", None)
    subnet_id = env.get("subnet_id", None)
    router_id = env.get("router_id", None)
    instance_id = env.get("instance_id", None)
    floatingip_id = env.get("floatingip_id", None)
    securitygroup_id = env.get("securitygroup_id", None)
    securitygroup_rule_id = env.get("securitygroup_rule_id", None)
    firewall_rule_id = env.get("firewall_rule_id", None)
    firewall_policy_id = env.get("firewall_policy_id", None)
    firewall_id = env.get("firewall_id", None)
    volume_id = env.get("volume_id", None)
    volume_ids = env.get("volume_ids", None)
    instance_ids = env.get("instance_ids", None)
    floatingip_ids = env.get("floatingip_ids", None)
    subnet_gw_id = env.get("subnet_gw_id", None)
    try:
        if instance_id:
            instance_delete(instance_id)
            instance_wait(instance_id, None)
            del env["instance_id"]
        if instance_ids:
            while len(instance_ids) > 0:
                id = instance_ids.pop()
                instance_delete(id)
                instance_wait(id, None)
        if volume_id:
            volume_delete(volume_id)
            del env["volume_id"]
        if volume_ids:
            while len(volume_ids) > 0:
                id = volume_ids.pop()
                volume_delete(id)
                time.sleep(2)
        if firewall_id:
            firewall_remove_all_router(firewall_id)
            firewall_delete(firewall_id)
            del env["firewall_id"]
        if firewall_rule_id and firewall_policy_id:
            try:
                firewall_polcy_remove_rule(firewall_policy_id, firewall_rule_id)
            except Exception, e:
                log("failed delete relationship for firewall and policy, may already deleted")
        if firewall_rule_id:
            firewall_rule_delete(firewall_rule_id)
            del env["firewall_rule_id"]
        if firewall_policy_id:
            firewall_policy_delete(firewall_policy_id)
            del env["firewall_policy_id"]
        if securitygroup_rule_id:
            securitygroup_rule_delete(securitygroup_rule_id)
            del env["securitygroup_rule_id"]
        if securitygroup_id:
            securitygroup_delete(securitygroup_id)
            del env["securitygroup_id"]
        if router_id and subnet_id:
            router_remove_internal_gw(router_id, subnet_id)
        if subnet_gw_id:
            router_remove_internal_gw(subnet_gw_id["router_id"], subnet_gw_id["subnet_id"])
            del env["subnet_gw_id"]
        if subnet_id:
            subnet_delete(subnet_id)
            del env["subnet_id"]
        time.sleep(2)
        if network_id:
            network_delete(network_id)
            del env["network_id"]
        if router_id:
            router_delete(router_id)
            del env["router_id"]
        if floatingip_id:
            floatingip_delete(floatingip_id)
            del env["floatingip_id"]
        if floatingip_ids:
            while len(floatingip_ids) > 0:
                id = floatingip_ids.pop()
                floatingip_delete(id)
    except Exception, e:
        log_main(str(env))
        raise e
    finally:
        log("=======================clear env end=================")


def refresh_arp_cache(ipaddr, router_id):
    time.sleep(2)
    port_id = port_find_router_external_port(router_id)
    qg_name = "qg-" + port_id[0:11]
    for controller_ip in ["10.160.60.72", "10.160.60.73", "10.160.60.74"]:
        cmd = 'ssh root@%s "ip netns exec qrouter-%s ip a | grep %s && ip netns exec qrouter-%s arping -A -I %s %s -c 3"'\
                % (controller_ip, router_id, ipaddr, router_id, qg_name, ipaddr)
        log(cmd)
        ssh = pexpect.spawn(cmd, timeout=120)
        i = ssh.expect(['password:', 'continue connecting (yes/no)?'], timeout=60)
        if i == 0:
            ssh.sendline("test")
        elif i == 1:
            ssh.sendline('yes\n')
            ssh.expect('password: ')
            ssh.sendline("test")
    return True


def cmd_ping(ipaddr, count=1):
    total = 0
    commands.getoutput("arp -d " + ipaddr)
    while total < count:
        time.sleep(5)
        cmd = "ping -c %s %s" % ("1", ipaddr)
        log(cmd)
        child = pexpect.spawn(cmd)
        index = child.expect(['0% packet loss', '100% packet loss',
                             pexpect.EOF, pexpect.TIMEOUT])
        log(child.before)
        if index == 0:
            log_main("ping failed count " + str(total))
            return True
        else:
            total = total + 1
    return False


def cmd_ping_between_two_ip(floating_ip, target_ip, passwd):
    cmd = "ping -c 1 %s  | grep packet\\ loss" % target_ip
    ssh = pexpect.spawn('ssh root@%s "%s"' % (floating_ip, cmd), timeout=120)
    log('ssh root@%s "%s"' % (floating_ip, cmd))
    i = ssh.expect(['password:', 'continue connecting (yes/no)?'], timeout=60)
    if i == 0:
        ssh.sendline(passwd)
    elif i == 1:
        ssh.sendline('yes\n')
        ssh.expect('password: ')
        ssh.sendline(passwd)
    ssh.sendline(cmd)
    result = ssh.read()
    log(result)
    index = result.find("0% packet loss")
    if index == -1:
        return False
    return True


def instance_wait_sshable(floating_ip):
    count = 0
    while count < 50:
        try:
            ssh = pexpect.spawn('ssh root@%s' % floating_ip, timeout=120)
            log("waiting for instance sshable")
            i = ssh.expect(['password:', 'continue connecting (yes/no)?'], timeout=60)
            if i == 0 or i == 1:
                log_main("ssh failed count:" + str(count))
                return
        except Exception, e:
            ssh.close()
            count = count + 1
            time.sleep(5)
    if count >= 50:
        raise Exception("ssh failed for 50 times!")
    log_main("ssh failed count:" + str(count))


def create_Networkenv():
    env = {}
    try:
        network_id, network_name = network_create()
        env["network_id"] = network_id
        env["network_name"] = network_name
        subnet_id = subnet_create(network_id)
        env["subnet_id"] = subnet_id
        router_id = router_create()
        env["router_id"] = router_id
        router_set_external_gw(router_id)
        router_add_internal_gw(router_id, subnet_id)
        securitygroup_id = securitygroup_create()
        env["securitygroup_id"] = securitygroup_id
        securitygroup_rule_id = securitygroup_rule_create(securitygroup_id)
        env["securitygroup_rule_id"] = securitygroup_rule_id
        second_securitygroup_rule_id = securitygroup_rule_create_tcp(securitygroup_id)

        firewall_rule_id = firewall_rule_create()
        env["firewall_rule_id"] = firewall_rule_id
        firewall_rule_id_tcp = firewall_rule_create_tcp()
        firewall_policy_id = firewall_policy_create()
        env["firewall_policy_id"] = firewall_policy_id
        firewall_polcy_insert_rule(firewall_policy_id, firewall_rule_id)
        firewall_polcy_insert_rule(firewall_policy_id, firewall_rule_id_tcp)
        firewall_id = firewall_create(firewall_policy_id)
        env["firewall_id"] = firewall_id
        firewall_change_router(firewall_id, router_id)
        firewall_wait(firewall_id, "ACTIVE")

        return env
    except Exception, e:
        log_main(traceback.format_exc())
        log_main("create env failed:" + str(e))
        clear_env(env)
        return {}


def testcase_basic():
    env = {}
    log("=======================testcase_basic start=================")
    try:
        network_id, network_name = network_create()
        env["network_id"] = network_id
        subnet_id = subnet_create(network_id)
        env["subnet_id"] = subnet_id
        router_id = router_create()
        env["router_id"] = router_id
        router_set_external_gw(router_id)
        router_add_internal_gw(router_id, subnet_id)
        securitygroup_id = securitygroup_create()
        env["securitygroup_id"] = securitygroup_id
        securitygroup_rule_id = securitygroup_rule_create(securitygroup_id)
        env["securitygroup_rule_id"] = securitygroup_rule_id

        firewall_rule_id = firewall_rule_create()
        env["firewall_rule_id"] = firewall_rule_id
        firewall_policy_id = firewall_policy_create()
        env["firewall_policy_id"] = firewall_policy_id
        firewall_polcy_insert_rule(firewall_policy_id, firewall_rule_id)
        firewall_id = firewall_create(firewall_policy_id)
        env["firewall_id"] = firewall_id
        firewall_change_router(firewall_id, router_id)
        firewall_wait(firewall_id, "ACTIVE")

        volume_id = volume_create()
        env["volume_id"] = volume_id
        volume_wait(volume_id, "available")

        instance_id = None
        if TEST_BOOT_VOLUME:
            instance_id = instance_create_by_volume(network_id, securitygroup_id, volume_id)
        else:
            instance_id = instance_create_by_image(network_id, securitygroup_id)
        env["instance_id"] = instance_id
        instance_wait(instance_id, "ACTIVE")

        instance_ip, instance_mac = instance_find_fix_ip(instance_id, network_name)
        port_id = port_find_by_mac(instance_mac)
        floatingip_id, floatingip_addr = floatingip_create(port_id)
        env["floatingip_id"] = floatingip_id
        refresh_arp_cache(floatingip_addr, router_id)
        if not cmd_ping(floatingip_addr, 20):
            log_main("basic test case failed due to ping failed")
            log_main(str(env))
            return False
        if TEST_KEEP_ENV:
            clear_env(env)
        return True
    except Exception, e:
        log_main(traceback.format_exc())
        log_main("test case failed:" + str(e))
        log_main(str(env))
        return False
    finally:
        log("=======================testcase_basic end=================")
        if not TEST_KEEP_ENV:
            clear_env(env)


def testcase_securitygroup():
    env = {}
    log("=======================testcase_securitygroup start=================")
    try:
        network_id, network_name = network_create()
        env["network_id"] = network_id
        subnet_id = subnet_create(network_id)
        env["subnet_id"] = subnet_id
        router_id = router_create()
        env["router_id"] = router_id
        router_set_external_gw(router_id)
        router_add_internal_gw(router_id, subnet_id)

        securitygroup_id = securitygroup_create()
        env["securitygroup_id"] = securitygroup_id

        volume_id = volume_create()
        env["volume_id"] = volume_id
        volume_wait(volume_id, "available")

        instance_id = None
        if TEST_BOOT_VOLUME:
            instance_id = instance_create_by_volume(network_id, securitygroup_id, volume_id)
        else:
            instance_id = instance_create_by_image(network_id, securitygroup_id)
        env["instance_id"] = instance_id
        instance_wait(instance_id, "ACTIVE")
        instance_ip, instance_mac = instance_find_fix_ip(instance_id, network_name)
        port_id = port_find_by_mac(instance_mac)
        floatingip_id, floatingip_addr = floatingip_create(port_id)
        env["floatingip_id"] = floatingip_id
        refresh_arp_cache(floatingip_addr, router_id)
        time.sleep(20)
        if cmd_ping(floatingip_addr, 5):
            log_main("security group test failed due to ping success")
            log_main(str(env))
            return False

        securitygroup_rule_id = securitygroup_rule_create(securitygroup_id)
        env["securitygroup_rule_id"] = securitygroup_rule_id
        if not cmd_ping(floatingip_addr, 20):
            log_main("security group test failed due to ping failed")
            log_main(str(env))
            return False

        securitygroup_rule_delete(securitygroup_rule_id)
        del env["securitygroup_rule_id"]
        time.sleep(20)
        if cmd_ping(floatingip_addr, 5):
            log_main("security group test failed due to ping success(2)")
            log_main(str(env))
            return False
        if TEST_KEEP_ENV:
            clear_env(env)
        return True
    except Exception, e:
        log_main(traceback.format_exc())
        log_main("test case failed:" + str(e))
        log_main(str(env))
        return False
    finally:
        log("=======================testcase_securitygroup end=================")
        if not TEST_KEEP_ENV:
            clear_env(env)


def testcase_fw():
    env = {}
    log("=======================testcase_fw start=================")
    try:
        network_id, network_name = network_create()
        env["network_id"] = network_id
        subnet_id = subnet_create(network_id)
        env["subnet_id"] = subnet_id
        router_id = router_create()
        env["router_id"] = router_id
        router_set_external_gw(router_id)
        router_add_internal_gw(router_id, subnet_id)
        securitygroup_id = securitygroup_create()
        env["securitygroup_id"] = securitygroup_id
        securitygroup_rule_id = securitygroup_rule_create(securitygroup_id)
        env["securitygroup_rule_id"] = securitygroup_rule_id

        firewall_rule_id = firewall_rule_create(False)
        env["firewall_rule_id"] = firewall_rule_id
        firewall_policy_id = firewall_policy_create()
        env["firewall_policy_id"] = firewall_policy_id
        firewall_polcy_insert_rule(firewall_policy_id, firewall_rule_id)
        firewall_id = firewall_create(firewall_policy_id)
        env["firewall_id"] = firewall_id
        firewall_change_router(firewall_id, router_id)
        firewall_wait(firewall_id, "ACTIVE")

        volume_id = volume_create()
        env["volume_id"] = volume_id
        volume_wait(volume_id, "available")

        instance_id = None
        if TEST_BOOT_VOLUME:
            instance_id = instance_create_by_volume(network_id, securitygroup_id, volume_id)
        else:
            instance_id = instance_create_by_image(network_id, securitygroup_id)
        env["instance_id"] = instance_id
        instance_wait(instance_id, "ACTIVE")
        instance_ip, instance_mac = instance_find_fix_ip(instance_id, network_name)
        port_id = port_find_by_mac(instance_mac)
        floatingip_id, floatingip_addr = floatingip_create(port_id)
        env["floatingip_id"] = floatingip_id
        refresh_arp_cache(floatingip_addr, router_id)
        if cmd_ping(floatingip_addr, 5):
            log_main("fw test failed due to ping success")
            log_main(str(env))
            return False

        firewall_rule_id_new = firewall_rule_create()
        env["firewall_rule_id"] = firewall_rule_id_new
        firewall_policy_id_new = firewall_policy_create()
        env["firewall_policy_id"] = firewall_policy_id_new
        firewall_polcy_insert_rule(firewall_policy_id_new, firewall_rule_id_new)

        firewall_change_policy(firewall_id, firewall_policy_id_new)

        firewall_polcy_remove_rule(firewall_policy_id, firewall_rule_id)
        firewall_rule_delete(firewall_rule_id)
        firewall_policy_delete(firewall_policy_id)
        firewall_wait(firewall_id, "ACTIVE")
        if not cmd_ping(floatingip_addr, 20):
            log_main("fw test failed due to ping failed")
            log_main(str(env))
            return False
        if TEST_KEEP_ENV:
            clear_env(env)
        return True
    except Exception, e:
        log_main(traceback.format_exc())
        log_main("test case failed:" + str(e))
        log_main(str(env))
        return False
    finally:
        log("=======================testcase_fw end=================")
        if not TEST_KEEP_ENV:
            clear_env(env)


def testcase_vm_mul_create_delete():
    log_main("=======================testcase_vm_mul_create_delete start=================")
    env = create_Networkenv()
    log_main(str(env))
    env["instances"] = []
    totalCreateVM = 0
    totalSuccess = 0
    while True:
        try:
            network_id = env.get("network_id", None)
            securitygroup_id = env.get("securitygroup_id", None)
            network_name = env.get("network_name", None)
            while len(env["instances"]) < 30:
                instance = []
                instance_id = instance_create_by_image(network_id, securitygroup_id)

                instance.append(instance_id)
                instance_wait(instance_id, "ACTIVE")
                totalCreateVM = totalCreateVM + 1

                instance_ip, instance_mac = instance_find_fix_ip(instance_id, network_name)
                port_id = port_find_by_mac(instance_mac)
                floatingip_id, floatingip_addr = floatingip_create(port_id)
                instance.append(floatingip_id)
                instance.append(floatingip_addr)
                if not cmd_ping(floatingip_addr, 20):
                    log_main("basic test case failed due to ping failed VM id:" + instance_id)
                    continue
                instance_wait_sshable(floatingip_addr)

                monitor_token = monitor_user_login()
                monitor_host_create(monitor_token, instance_id, instance_ip)

                env["instances"].insert(0,instance)
                totalSuccess = totalSuccess + 1
                log_main("create vm success :" + str(totalSuccess))
            log_main("total created vm:" + str(totalCreateVM) + " totalSuccess:" + str(totalSuccess))
            time.sleep(180)
            instance_to_delete = env["instances"].pop()
            instance_wait_sshable(instance_to_delete[2])

            instance_stop(instance_to_delete[0])
            instance_wait(instance_to_delete[0], "SHUTOFF")

            instance_start(instance_to_delete[0])
            instance_wait(instance_to_delete[0], "ACTIVE")
            instance_wait_sshable(instance_to_delete[2])

            instance_resize(instance_to_delete[0], "4")
            instance_wait(instance_to_delete[0], "VERIFY_RESIZE")
            instance_resize_confirm(instance_to_delete[0])
            instance_wait(instance_to_delete[0], "ACTIVE")
            instance_wait_sshable(instance_to_delete[2])
            
            volume_id = volume_create()
            instance_volume_attach(instance_id, volume_id)

            host_old = instance_find_host(instance_to_delete[0])
            instance_live_migrate(instance_to_delete[0])
            instance_wait(instance_to_delete[0], "ACTIVE")
            instance_wait_sshable(instance_to_delete[2])
            host_new = instance_find_host(instance_to_delete[0])
            log_main("old host:" + host_old)
            log_main("new host:" + host_new)
            if host_old == host_new:
                log_main("test case failed: live migration failed:" + str(instance_to_delete))
                continue

            instance_delete(instance_to_delete[0])
            instance_wait(instance_to_delete[0], None)
            floatingip_delete(instance_to_delete[1])
            log_main("delete vm:" + str(instance_to_delete))
        except Exception, e:
            log_main(traceback.format_exc())
            log_main("test case failed:" + str(e))
            log_main(str(env))


def testcase_connectivity_between_two_vlan():
    env_main = {}
    env_minor = {}
    log("=======================testcase_connectivity_between_two_vlan start=================")
    try:
        network_id, network_name = network_create()
        env_main["network_id"] = network_id
        subnet_id = subnet_create(network_id)
        env_main["subnet_id"] = subnet_id
        router_id = router_create()
        env_main["router_id"] = router_id
        router_set_external_gw(router_id)
        router_add_internal_gw(router_id, subnet_id)
        securitygroup_id = securitygroup_create()
        env_main["securitygroup_id"] = securitygroup_id
        securitygroup_rule_id = securitygroup_rule_create(securitygroup_id)
        env_main["securitygroup_rule_id"] = securitygroup_rule_id

        volume_id = volume_create()
        env_main["volume_id"] = volume_id
        volume_wait(volume_id, "available")

        instance_id = None
        if TEST_BOOT_VOLUME:
            instance_id = instance_create_by_volume(network_id, securitygroup_id, volume_id)
        else:
            instance_id = instance_create_by_image(network_id, securitygroup_id)
        env_main["instance_id"] = instance_id
        instance_wait(instance_id, "ACTIVE")

        instance_ip, instance_mac = instance_find_fix_ip(instance_id, network_name)
        port_id = port_find_by_mac(instance_mac)
        floatingip_id, floatingip_addr = floatingip_create(port_id)
        env_main["floatingip_id"] = floatingip_id
        refresh_arp_cache(floatingip_addr, router_id)
        if not cmd_ping(floatingip_addr, 20):
            log_main("basic test case failed due to frist vm ping failed")
            log_main(str(env_minor))
            log_main(str(env_main))
            return False

        second_securitygroup_rule_id = securitygroup_rule_create_tcp(securitygroup_id)
        env_minor["securitygroup_rule_id"] = second_securitygroup_rule_id

        second_network_id, second_network_name = network_create()
        env_minor["network_id"] = second_network_id
        second_subnet_id = subnet_create(second_network_id, '192.168.1.0/24')
        env_minor["subnet_id"] = second_subnet_id
        router_add_internal_gw(router_id, second_subnet_id)
        env_minor["subnet_gw_id"] = {"router_id": router_id, "subnet_id": second_subnet_id}

        second_volume_id = volume_create()
        env_minor["volume_id"] = second_volume_id
        volume_wait(second_volume_id, "available")

        second_instance_id = None
        if TEST_BOOT_VOLUME:
            second_instance_id = instance_create_by_volume(second_network_id, securitygroup_id, second_volume_id)
        else:
            second_instance_id = instance_create_by_image(second_network_id, securitygroup_id)
        env_minor["instance_id"] = second_instance_id
        instance_wait(second_instance_id, "ACTIVE")

        second_instance_ip, second_instance_mac = instance_find_fix_ip(second_instance_id, second_network_name)
        second_port_id = port_find_by_mac(second_instance_mac)
        second_floatingip_id, second_floatingip_addr = floatingip_create(second_port_id)
        env_minor["floatingip_id"] = second_floatingip_id
        refresh_arp_cache(second_floatingip_addr, router_id)
        if not cmd_ping(second_floatingip_addr, 20):
            log_main("basic test case failed due to second vm ping failed")
            log_main(str(env_minor))
            log_main(str(env_main))
            return False

        instance_wait_sshable(floatingip_addr)

        if not cmd_ping_between_two_ip(floatingip_addr, second_instance_ip, TEST_GUESTOS_PASSWD):
            log_main("ping failed between first and second vm")
            log_main(str(env_minor))
            log_main(str(env_main))
            return False

        if TEST_KEEP_ENV:
            clear_env(env_minor)
            clear_env(env_main)
        return True
    except Exception, e:
        log_main(traceback.format_exc())
        log_main("test case failed:" + str(e))
        log_main(str(env_minor))
        log_main(str(env_main))
        return False
    finally:
        log("=======================testcase_connectivity_between_two_vlan end=================")
        if not TEST_KEEP_ENV:
            clear_env(env_minor)
            clear_env(env_main)


def run_case(case):
    if case():
        drop_log()
        return True
    else:
        print_log()
        return False


def run():
    i = 0
    basic_success = 0
    sg_success = 0
    fw_success = 0
    connectivity = 0
    while i < TEST_RUN_COUNT:
        i = i+1
        print "===round %d %s====" % (i, time.strftime(LOG_TIME_FORMAT, time.localtime()))
        if run_case(testcase_basic):
            print "testcase_basic success!"
            basic_success = basic_success + 1
        elif TEST_KEEP_ENV:
            break
        if run_case(testcase_securitygroup):
            print "testcase_securitygroup success!"
            sg_success = sg_success + 1
        elif TEST_KEEP_ENV:
            break
        if run_case(testcase_fw):
            print "testcase_fw success!"
            fw_success = fw_success + 1
        elif TEST_KEEP_ENV:
            break
        if run_case(testcase_connectivity_between_two_vlan):
            print "testcase_connectivity_between_two_vlan success!"
            connectivity = connectivity + 1
        elif TEST_KEEP_ENV:
            break
    print "====result===="
    print "total:" + str(i)
    print "basic success:" + str(basic_success)
    print "sg success:" + str(sg_success)
    print "fw success:" + str(fw_success)
    print "connect success:" + str(connectivity)


#run()
run_case(testcase_vm_mul_create_delete)
