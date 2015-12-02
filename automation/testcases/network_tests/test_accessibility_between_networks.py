# !/usr/bin/python
import pexpect
import traceback
import uuid

from automation.common import exceptions
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.compute_tests.base_compute import Base as compute_base
from testcases.network_tests.base import Base

from time import sleep

CONF = config.CONF
LOG = logging.getLogger(__name__)


class AccessibilityNetworksTest(Base):

    def __init__(self):
        try:
            random = str(uuid.uuid4())
            network_name = "net-" + random
            router_name = "router-" + random
            sec_group_name = "sg-" + random
            subnet_name = "subnet" + random
            firewall_name = "firewall-" + random
            server_name = "server-" + random
            self.net_base = Base(network_name=network_name,
                                 router_name=router_name,
                                 sec_group_name=sec_group_name,
                                 subnet_name=subnet_name,
                                 firewall_name=firewall_name)
            self.compute_base = compute_base(flavor_ref=None,
                                             image_ref=None,
                                             network_id=None,
                                             security_group_id=None,
                                             server_name=server_name)
        except Exception as e:
            LOG.warning(_LW('Init test env failed, msg: %(msg)s, traceback: '
                            '%(tb)s.'), {'msg': e,
                                         'tb': traceback.format_exc()})
            raise exceptions.InitFailed

    def test_accessibility_between_networks(self):
        try:
            router = self.net_base.create_router()
            network01 = self.net_base.create_network()
            network02 = self.net_base.create_network()
            subnet01 = self.net_base.create_subnet(network01['id'])
            subnet02 = self.net_base.create_subnet(network02['id'])

            self.net_base.router_set_external_gw(router['id'])
            self.net_base.router_add_subnet(router['id'], subnet01['id'])
            self.net_base.router_add_subnet(router['id'], subnet02['id'])

            sec_grp = self.net_base.create_security_group()
            self.net_base.create_security_group_rule(sec_grp['id'], 'IPv4',
                                                     'ingress',
                                                     '0.0.0.0/0',
                                                     'ICMP')
            self.net_base.create_security_group_rule(sec_grp['id'], 'IPv4',
                                                     'ingress',
                                                     '0.0.0.0/0',
                                                     'TCP')
            self.net_base.create_security_group_rule(sec_grp['id'], 'IPv4',
                                                     'egress',
                                                     '0.0.0.0/0',
                                                     'TCP')
        except Exception as e:
            LOG.warning(_LW('Create network env failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            self.net_base.delete_all()
            raise e

        # create instance01 on network01
        try:
            instance01 = self.compute_base.\
                create_server(name=None, image_id=None, flavor_id=None,
                              network_id=network01['id'],
                              sec_grp_id=sec_grp['id'])
            interfaces_first = self.compute_base.\
                show_server_interfaces(instance01['id'])
            interfaces_first = interfaces_first.get('interfaceAttachments', [])
            if not interfaces_first:
                LOG.warning(_LW('During network test server has no port!'))
                raise
            port_id_first = interfaces_first[0]['port_id']
            floatingip = self.net_base.create_floating_ip()
            floatingip_addr = floatingip['floating_ip_address']
            self.net_base.associate_floating_ip(floatingip['id'],
                                                port_id_first)

            # test the floating connectable
            self.net_base.test_ping_floatingip(floatingip_addr)

            # create instance02 on network02
            instance02 = self.compute_base.\
                create_server(name=None, image_id=None, flavor_id=None,
                              network_id=network02['id'],
                              sec_grp_id=sec_grp['id'])
            interfaces_sec = self.compute_base.\
                show_server_interfaces(instance02['id'])
            interfaces_sec = interfaces_sec.get('interfaceAttachments', [])
            if not interfaces_sec:
                LOG.warning(_LW('During network test server has no port!'))
                raise
            # port_id_second = interfaces_sec[0]['port_id']
        except Exception as e:
            LOG.warning(_LW('Test network accessibility failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})

            is_successfully_deleted = True
            try:
                self.compute_base.delete_all()
            except Exception as e:
                LOG.warning(_LW('Compute clear process failed.'))
                is_successfully_deleted = False
            try:
                self.net_base.delete_all()
            except Exception as e:
                LOG.warning(_LW('Network clear process failed.'))
                is_successfully_deleted = False

            if not is_successfully_deleted:
                raise exceptions.DeleteException

        # get the fixedip for instance02
        try:
            instance02_details = \
                self.compute_base.show_server(instance02['id'])
            network02_name = network02['name']
            instance02_fixedip = \
                instance02_details["addresses"][network02_name][0]["addr"]
            # self.net_base.test_ping_floatingip(instance02_fixedip)
        except Exception as e:
            LOG.warning(_LW('error happened when try to ping vm2 from vm1, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})

            is_successfully_deleted = True
            try:
                self.compute_base.delete_all()
            except Exception as e:
                LOG.warning(_LW('Network clear process failed.'))
                is_successfully_deleted = False

            try:
                self.net_base.delete_all()
            except Exception as e:
                LOG.warning(_LW('Compute clear process failed.'))
                is_successfully_deleted = False
            if not is_successfully_deleted:
                raise exceptions.DeleteException

        # begin to ping from server01 to server02
        try:
            admin_pass = CONF.compute.admin_password

            if not self.test_ip_accessibility(floatingip_addr,
                                              instance02_fixedip,
                                              admin_pass,
                                              interval=None,
                                              timeout=None):
                LOG.info("ping failed between first and second vm")
                return False
            LOG.info("ping successfully from first vm to second vm.")
        except Exception as e:
            LOG.warning(_LW('Test network ssh server01 failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise e
        finally:
            is_successfully_deleted = True
            try:
                self.compute_base.delete_all()
            except Exception as e:
                LOG.warning(_LW('Compute clear process failed.'))
                is_successfully_deleted = False

            try:
                self.net_base.delete_all()
            except Exception as e:
                LOG.warning(_LW('Network clear process failed.'))
                is_successfully_deleted = False

            if not is_successfully_deleted:
                raise exceptions.DeleteException

    def _cmd_ping_between_two_ip(self, floating_ip, target_ip, passwd):
        cmd = "ping -c 1 %s  | grep packet\\ loss" % target_ip
        ssh = pexpect.spawn('ssh root@%s "%s"' % (floating_ip, cmd),
                            timeout=120)
        LOG.info('ssh root@%s "%s"' % (floating_ip, cmd))
        i = ssh.expect(['password:', 'continue connecting (yes/no)?'],
                       timeout=60)
        if i == 0:
            ssh.sendline(passwd)
        elif i == 1:
            ssh.sendline('yes\n')
            ssh.expect('password: ')
            ssh.sendline(passwd)
        ssh.sendline(cmd)
        result = ssh.read()
        LOG.info(result)
        index1 = result.find(", 0% packet loss")
        # index2 = result.find("errors")
        if index1 == -1:
            return False
        return True

    def test_ip_accessibility(self, floating_ip, target_ip, passwd,
                              interval=None, timeout=None):
        if not interval:
            interval = CONF.network.build_interval
        if not timeout:
            timeout = CONF.network.build_timeout
        while timeout >= 0:
            timeout -= interval
            if self._cmd_ping_between_two_ip(floating_ip, target_ip, passwd):
                return True
            sleep(interval)
        raise exceptions.TimeoutException

if __name__ == '__main__':
    network_accessibility_test = AccessibilityNetworksTest()
    network_accessibility_test.test_accessibility_between_networks()
