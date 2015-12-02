# !/usr/bin/python

import json
import requests

from automation.common import exceptions
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging

CONF = config.CONF
LOG = logging.getLogger(__name__)


class MonitorBase(object):

    def __init__(self, server=None, user=None, passwd=None):
        self.server = server if server else CONF.compute.zabbix_server
        self.user = user if user else CONF.compute.zabbix_user
        self.password = passwd if passwd else CONF.compute.zabbix_password
        self.base_url = 'http://%s:8090/zabbix/api_jsonrpc.php' % self.server
        self.session = requests.session()
        self.session.auth = (self.user, self.password)
        self.session.verify = False
        self.session.headers.update({"Content-Type": "application/json"})

    def login_monitor(self):
        data = \
            {
                "jsonrpc": "2.0",
                "method": "user.login",
                "params":
                    {
                        "user": self.user,
                        "password": self.password
                    },
                "id": 1
            }
        response = self.session.post(self.base_url, data=json.dumps(data))
        if response.status_code == 200:
            return response.json()['result']
        else:
            LOG.warning(_LW("Failed to login the monitor system"))
            return None

    def create_monitor(self, server_id):
        auth_id = self.login_monitor()
        if auth_id is None:
            raise exceptions.CreateMonitorException()
        # In the body, ip is useless but is needed, so i fake it
        data = \
            {
                "jsonrpc": "2.0",
                "method": "host.create",
                "params": {
                    "host": server_id,
                    "interfaces": [
                        {
                            "type": 1,
                            "main": 1,
                            "useip": 1,
                            "ip": "192.168.0.1",
                            "dns": "",
                            "port": "10050"
                        }
                    ],
                    "groups": [{"groupid": "2"}],
                    "templates": [{"templateid": "10002"}],
                },
                "auth": auth_id,
                "id": 1,
            }
        response = self.session.post(self.base_url, data=json.dumps(data))
        if response.status_code == 200:
            return
        else:
            LOG.warning(_LW("Failed to create monitor of server %s"),
                        server_id)
            raise exceptions.CreateMonitorException()
