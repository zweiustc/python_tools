# !/usr/bin/python
import traceback
import uuid

from automation.common import exceptions
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.compute_tests.base_compute import Base as compute_base
from testcases.network_tests.base import Base as network_base

CONF = config.CONF
LOG = logging.getLogger(__name__)


if __name__ == '__main__':
    try:
        random = str(uuid.uuid4())
        network_name = "net-" + random
        router_name = "router-" + random
        sec_group_name = "sg-" + random
        subnet_name = "subnet-" + random
        firewall_name = "firewall-" + random
        server_name = "server-" + random
        network = network_base(network_name=network_name,
                               router_name=router_name,
                               sec_group_name=sec_group_name,
                               subnet_name=subnet_name,
                               firewall_name=firewall_name)
    except Exception as e:
        LOG.warning(_LW('Init test env failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
        raise exceptions.InitFailed

    try:
        network_created = network.create_network()
        network_id = network_created['id']
        subnet_created = network.create_subnet(network_id)
        subnet_id = subnet_created['id']
        router_created = network.create_router()
        router_id = router_created['id']
        network.router_set_external_gw(router_id)
        network.router_add_subnet(router_id, subnet_id)
        security_group_created = network.create_security_group()
        security_group_id = security_group_created['id']
        network.create_security_group_rule(
            security_group_id, 'IPv4', 'ingress', '0.0.0.0/0', 'ICMP')
    except Exception as e:
        LOG.warning(_LW('Test create network failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
        network.delete_all()
        raise e

    try:
        server = compute_base(network_id=network_id,
                              security_group_id=security_group_id,
                              server_name=server_name)
    except Exception as e:
        LOG.warning(_LW('Init test env failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
        network.delete_all()
        raise exceptions.InitFailed

    try:
        server_created = server.create_server()
        server_id = server_created['id']
        server_details = server.show_server(server_id)
    except Exception as e:
        LOG.warning(_LW('Test create server failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
        try:
            server.delete_all()
        except Exception as e:
            LOG.warning(_LW('During network basic test server delete failed.'))
        try:
            network.delete_all()
        except Exception as e:
            LOG.warning(_LW('During network basic test net delete failed.'))
        raise e

    try:
        server_interfaces = server.show_server_interfaces(server_id)
        server_interfaces = server_interfaces.get('interfaceAttachments', [])
        if not server_interfaces:
            LOG.warning(_LW('During network test server has no port!'))
            raise exceptions.NotFound
        port_id = server_interfaces[0]['port_id']
        floatingip_created = network.create_floating_ip()
        network.associate_floating_ip(floatingip_created['id'], port_id)
        ip_addr = floatingip_created['floating_ip_address']
        network.test_ping_floatingip(ip_addr)
    except Exception as e:
        LOG.warning(_LW('Test basic network failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
        raise e
    finally:
        is_successfully_deleted = True
        try:
            server.delete_all()
        except Exception as e:
            LOG.warning(_LW('During network basic test server delete failed.'))
            is_successfully_deleted = False

        try:
            network.delete_all()
        except Exception as e:
            LOG.warning(_LW('Network clear process failed.'))
            is_successfully_deleted = False

        if not is_successfully_deleted:
            raise exceptions.DeleteException
