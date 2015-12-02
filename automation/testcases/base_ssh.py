# !/usr/bin/python
from oslo_log import log as logging
import paramiko

LOG = logging.getLogger(__name__)


class BaseSSH(object):
    @staticmethod
    def run_cmd(host, passwd, cmd):
        try:
            client = paramiko.SSHClient()
            client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            client.connect(hostname=host, username='root', password=passwd)

            stdin, stdout, stderr = client.exec_command(cmd)
            err = stderr.read()
            if err:
                return False
            return stdout.read()
        except paramiko.ssh_exception.AuthenticationException:
            return False
