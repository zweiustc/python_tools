# !/usr/bin/python
import traceback

from automation.common import exceptions
from automation import config
from automation.i18n import _LI
from automation.i18n import _LW
from oslo_log import log as logging
import random
from testcases.compute_tests.base_compute import Base as Compute_base
from testcases.network_tests.base import Base

import uuid

CONF = config.CONF
LOG = logging.getLogger(__name__)


class MultipleVMTest(Base):

    def __init__(self):
        try:
            super(MultipleVMTest, self).__init__()
            random = str(uuid.uuid4())[:5]
            network_name = "net-" + random
            router_name = "router-" + random
            sec_group_name = "sg-" + random
            self.net_base = Base(network_name=network_name,
                                 router_name=router_name,
                                 sec_group_name=sec_group_name)
            self.compute = Compute_base()
        except Exception as e:
            LOG.warning(_LW('Init test env failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
            raise exceptions.InitFailed

    def test_multipleVM(self):
        try:
            network = self.create_network()
            subnet = self.create_subnet(network.get('id'))
            router = self.create_router()
            self.router_set_external_gw(router.get('id'))
            self.router_add_subnet(router.get('id'), subnet.get('id'))
            sg = self.create_security_group()
            self.create_security_group_rule(sg.get('id'),
                                            'IPv4',
                                            'ingress',
                                            '0.0.0.0/0',
                                            'ICMP'
                                            )
        except Exception as e:
            LOG.warning(_LW('Create network env failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            self.delete_all()
            raise e

        try:
            success_count = 0
            vm_count = random.randint(1, 10)
            for i in range(vm_count):
                try:
                    LOG.info(_LI('Create and delete %(i)d of %(vm_count)d vm'
                                 ' start'), {'i': i + 1, 'vm_count': vm_count})
                    vm_name = "server-" + str(uuid.uuid4())[:5]
                    server = self.compute.\
                        create_server(name=vm_name,
                                      network_id=network.get('id'),
                                      sec_grp_id=sg.get('id'))
                    interfaces = self.compute.\
                        show_server_interfaces(server['id'])
                    interfaces = interfaces.get('interfaceAttachments', [])
                    if not interfaces:
                        LOG.warning(_LW('During network test server '
                                        'has no port!'))
                        raise
                    port_id = interfaces[0]['port_id']
                    floatingip = self.create_floating_ip(port_id)
                    floatingip_addr = floatingip['floating_ip_address']
                    self.test_ping_floatingip(floatingip_addr)
                    success_count += 1
                    LOG.info(_LI('Create and delete %(i)d of %(vm_count)d vm'
                                 ' success'),
                             {'i': i + 1,
                              'vm_count': vm_count})
                except Exception as e:
                    LOG.warning(_LW('Create and delete %(i)d of %(vm_count)d '
                                    'vm failed, msg: %(msg)s, '
                                    'traceback: %(tb)s.'),
                                {'i': i + 1, 'vm_count': vm_count,
                                 'msg': e, 'tb': traceback.format_exc()})

            LOG.info(_LI(' %(success_count)d success of %(vm_count)d round '
                         'total'),
                     {'success_count': success_count,
                      'vm_count': vm_count})
        except Exception as e:
            LOG.warning(_LW('Test multiple vm failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
        finally:
            is_successfully_deleted = True
            try:
                self.compute.delete_all()
            except Exception as e:
                LOG.warning(_LW('Clear compute resource failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                            {'msg': e, 'tb': traceback.format_exc()})
                is_successfully_deleted = False

            try:
                self.delete_all()
            except Exception as e:
                LOG.warning(_LW('Clear network resource failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                            {'msg': e, 'tb': traceback.format_exc()})
                is_successfully_deleted = False

            if not is_successfully_deleted:
                raise exceptions.DeleteException

if __name__ == '__main__':
    nt = MultipleVMTest()
    nt.test_multipleVM()
