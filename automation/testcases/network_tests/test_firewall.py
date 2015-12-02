# !/usr/bin/python
import traceback
import uuid

from automation.common import exceptions
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.compute_tests.base_compute import Base as compute_base
from testcases.network_tests.base import Base


CONF = config.CONF
LOG = logging.getLogger(__name__)


class FirewallTest(Base):

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

    def test_firewall(self):
        try:
            router = self.net_base.create_router()
            network = self.net_base.create_network()
            subnet = self.net_base.create_subnet(network['id'])
            self.net_base.router_set_external_gw(router['id'])
            self.net_base.router_add_subnet(router['id'], subnet['id'])
            sec_grp = self.net_base.create_security_group()
            self.net_base.create_security_group_rule(sec_grp['id'], 'IPv4',
                                                     'ingress',
                                                     '0.0.0.0/0',
                                                     'ICMP')

            firewall_rule = self.net_base.create_firewall_rule(True, None)
            firewall_policy = self.net_base.create_firewall_policy()
            self.net_base.insert_firewall_policy_rule(firewall_policy['id'],
                                                      firewall_rule['id'])
            firewall = self.net_base.create_firewall(firewall_policy['id'])
            self.net_base.change_firewall_router(firewall['id'], router['id'])
        except Exception as e:
            LOG.warning(_LW('Create network env failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            self.net_base.delete_all()
            raise e

        try:
            instance = self.compute_base.\
                create_server(name=None, image_id=None, flavor_id=None,
                              network_id=network['id'],
                              sec_grp_id=sec_grp['id'])
            interfaces = self.compute_base.\
                show_server_interfaces(instance['id'])
            interfaces = interfaces.get('interfaceAttachments', [])
            if not interfaces:
                LOG.warning(_LW('During network test server has no port!'))
                raise
            port_id = interfaces[0]['port_id']
            floatingip = self.net_base.create_floating_ip()
            floatingip_addr = floatingip['floating_ip_address']
            self.net_base.associate_floating_ip(floatingip['id'], port_id)

            # test the floating connectable
            self.net_base.test_ping_floatingip(floatingip_addr)
        except Exception as e:
            LOG.warning(_LW('Test basic network failed, msg: %(msg)s, '
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


if __name__ == '__main__':
    firewall_test = FirewallTest()
    firewall_test.test_firewall()
