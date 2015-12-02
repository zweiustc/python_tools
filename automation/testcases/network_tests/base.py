# !/usr/bin/python
import random
import subprocess
import time
import traceback
import uuid

from automation.clients import Manager
from automation.common import exceptions
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging

CONF = config.CONF
LOG = logging.getLogger(__name__)


class Base(object):
    def __init__(self, network_name=None, vlan_id=None, subnet_cidr=None,
                 subnet_ip_version=None, router_name=None,
                 external_network_id=None, sec_group_name=None,
                 subnet_name=None, firewall_name=None):
        self.manager = Manager()
        self.network_client = self.manager.network_client
        self.network_name = network_name or CONF.network.tenant_network_name
        self.subnet_name = subnet_name or CONF.network.tenant_subnet_name
        self.firewall_name = firewall_name or CONF.network.firewall_name
        self.vlan_id = vlan_id
        self.subnet_cidr = subnet_cidr or CONF.network.subnet_cidr
        self.subnet_ip_version = \
            subnet_ip_version or CONF.network.subnet_ip_version
        self.router_name = router_name or CONF.network.router_name
        self.external_network_id = \
            external_network_id or self._get_external_network()
        self.sec_group_name = sec_group_name or CONF.network.sec_group_name
        self.networks = list()
        self.subnets = list()
        self.routers = list()
        self.security_groups = list()
        self.security_group_rules = list()
        self.floating_ips = list()
        self.firewall_rules = list()
        self.firewall_policies = list()
        self.firewalls = list()

    def create_network(self, name=None):
        network_name = name if name is not None else self.network_name
        params = {
            'name': network_name,
            'provider:network_type': 'vlan',
            'provider:physical_network': 'default',
            'admin_state_up': True
        }
        if self.vlan_id:
            params.update({'provider:segmentation_id': self.vlan_id})
        network_created = self.network_client.create_network(**params)
        self.networks.append(network_created['network']['id'])
        self.wait_resource_status(self._get_network_status,
                                  network_created['network']['id'],
                                  'ACTIVE')
        return network_created['network']

    def _get_network_status(self, resource_id):
        detail = self.network_client.show_network(resource_id)
        return detail['network']['status']

    def create_subnet(self, network_id, name=None, subnet_cidr=None,
                      ip_version=None):
        if subnet_cidr:
            cidr = subnet_cidr
        else:
            init_cidr = "192.168.1.0/24"
            sector_a, sector_b, sector_c, sector_d = init_cidr.split('.')
            sector_c = random.randint(2, 253)
            random_cidr = \
                sector_a + "." + sector_b + "." + str(sector_c) + "." +\
                sector_d
            cidr = random_cidr
        # cidr = subnet_cidr if subnet_cidr else self.subnet_cidr
        version = ip_version if ip_version else self.subnet_ip_version
        subnet_name = name if name else self.subnet_name
        params = {
            'name': subnet_name,
            'network_id': network_id,
            'ip_version': version,
            'cidr': cidr
        }
        subnet_created = self.network_client.create_subnet(**params)
        self.subnets.append(subnet_created['subnet']['id'])
        return subnet_created['subnet']

    def create_firewall_rule(self, connectable=True, name=None,
                             src_ip_addr='0.0.0.0/0',
                             dest_ip_addr='0.0.0.0/0',
                             protocol='icmp'):
        action = 'allow' if connectable else 'deny'
        rule_name = name or uuid.uuid4()
        kwargs = \
            {
                "protocol": protocol,
                "name": rule_name,
                "enabled": "True",
                "source_ip_address": src_ip_addr,
                "destination_ip_address": dest_ip_addr,
                "action": action
            }
        rule_created = \
            self.network_client.create_firewall_rule(**kwargs)
        self.firewall_rules.\
            append(rule_created['firewall_rule']['id'])
        return rule_created['firewall_rule']

    def delete_firewall_rule(self, firewall_rule_id):
        self.network_client.delete_firewall_rule(firewall_rule_id)
        self.firewall_rules.remove(firewall_rule_id)

    def create_firewall_policy(self, name=None):
        policy_name = name or uuid.uuid4()
        kwargs = {'name': policy_name}
        policy_created = \
            self.network_client.create_firewall_policy(**kwargs)
        self.firewall_policies.\
            append(policy_created['firewall_policy']['id'])
        return policy_created['firewall_policy']

    def delete_firewall_policy(self, firewall_policy_id):
        self.network_client.delete_firewall_policy(firewall_policy_id)
        self.firewall_policies.remove(firewall_policy_id)
        self.wait_resource_status(self._get_firewall_status,
                                  firewall_policy_id, 'ACTIVE')

    def create_firewall(self, firewall_policy_id, name=None):
        firewall_name = name if name else self.firewall_name
        if firewall_name is None:
            firewall_name = str(uuid.uuid4())
        kwargs = \
            {
                'firewall_policy_id': firewall_policy_id,
                'admin_state_up': True,
                'router_ids': [],
                'name': firewall_name
            }
        firewall_created = self.network_client.\
            create_firewall(**kwargs)
        self.firewalls.append(firewall_created['firewall']['id'])
        return firewall_created['firewall']

    def delete_firewall(self, firewall_id):
        self.network_client.delete_firewall(firewall_id)
        self.firewalls.remove(firewall_id)

    def insert_firewall_policy_rule(self, firewall_policy_id,
                                    firewall_rule_id):
        return self.network_client.\
            insert_firewall_policy_rule(firewall_policy_id,
                                        firewall_rule_id)

    def remove_firewall_policy_rule(self, firewall_policy_id,
                                    firewall_rule_id):
        return self.network_client.\
            firewall_policy_remove_rule(firewall_policy_id,
                                        firewall_rule_id)

    def change_firewall_router(self, firewall_id, router_id):
        kwargs = {'router_ids': [router_id]}
        self.network_client.update_firewall(firewall_id, **kwargs)
        self.wait_resource_status(self._get_firewall_status,
                                  firewall_id, 'ACTIVE')

    def change_firewall_policy(self, firewall_id, firewall_policy_id):
        kwargs = {'firewall_policy_id': firewall_policy_id}
        self.network_client.update_firewall(firewall_id, **kwargs)

    def remove_firewall_all_routers(self, firewall_id):
        kwargs = {'router_ids': []}
        self.network_client.update_firewall(firewall_id, **kwargs)

    def _get_firewall_status(self, firewall_id):
        detail = self.network_client.show_firewall(firewall_id)
        return detail['firewall']['status']

    def create_router(self, name=None):
        router_name = name if name else self.router_name
        router_created = \
            self.network_client.create_router(router_name)
        self.routers.append(router_created['router']['id'])
        self.wait_resource_status(self._get_router_status,
                                  router_created['router']['id'],
                                  'ACTIVE')
        return router_created['router']

    def _get_router_status(self, resource_id):
        detail = self.network_client.show_router(resource_id)
        return detail['router']['status']

    def router_set_external_gw(self, router_id):
        params = {
            "external_gateway_info": {"network_id": self.external_network_id}
        }
        router = self.network_client.update_router(router_id, **params)
        return router['router']

    def router_unset_external_gw(self, router_id):
        params = {
            "external_gateway_info": {}
        }
        router = self.network_client.update_router(router_id, **params)
        return router['router']

    def _get_external_network(self, external=CONF.network.external_network):
        networks_resp = self.network_client.list_networks()
        networks = networks_resp.get('networks', [])
        for network in networks:
            if network['name'] == external:
                return network['id']
        LOG.warning(_LW('Specific network not found %s.'), external)
        raise exceptions.NetworkNotFound

    def router_add_subnet(self, router_id, subnet_id):
        router = self.network_client.\
            add_router_interface_with_subnet_id(router_id, subnet_id)
        return router

    def router_remove_subnet(self, router_id, subnet_id):
        """Response body:
        {
        "subnet_id": "918dfbea-8b0f-45f3-b3e6-55251160d40c",
        "tenant_id": "b978b4bdd60f486f83c5652c40773603",
        "subnet_ids": ["918dfbea-8b0f-45f3-b3e6-55251160d40c"],
        "port_id": "d0be5d31-91b7-4757-ad89-2aa400bb068c",
        "id": "1f3b82a1-3724-484b-aa5f-6cad249de9c8"}
        """
        router = self.network_client.\
            remove_router_interface_with_subnet_id(router_id, subnet_id)
        return router

    def create_security_group(self, name=None):
        params = {
            'name': name if name else self.sec_group_name
        }
        sec = self.network_client.create_security_group(**params)
        self.security_groups.append(sec['security_group']['id'])
        return sec['security_group']

    def create_security_group_rule(self, security_group_id, ethertype,
                                   direction, remote_ip_prefix, protocol):
        params = {
            "ethertype": ethertype,
            "direction": direction,
            "remote_ip_prefix": remote_ip_prefix,
            "protocol": protocol,
            "security_group_id": security_group_id
        }
        rule = self.network_client.create_security_group_rule(**params)
        self.security_group_rules.append(rule['security_group_rule']['id'])
        return rule['security_group_rule']

    def create_floating_ip(self, port_id=None):
        wait_status = 'DOWN'
        params = {
            "floating_network_id": self.external_network_id
        }
        if port_id:
            params['port_id'] = port_id
            wait_status = 'ACTIVE'
        ip = self.network_client.create_floatingip(**params)
        self.floating_ips.append(ip['floatingip']['id'])
        self.wait_resource_status(self._get_floatingip_status,
                                  ip['floatingip']['id'],
                                  wait_status)
        return ip['floatingip']

    def associate_floating_ip(self, floatingip_id, port_id):
        params = dict()
        params['port_id'] = port_id
        ip = self.network_client.update_floatingip(floatingip_id, **params)
        self.wait_resource_status(self._get_floatingip_status,
                                  ip['floatingip']['id'],
                                  'ACTIVE')
        return ip['floatingip']

    def disassociate_floating_ip(self, floatingip_id):
        params = {'port_id': None}
        ip = self.network_client.update_floatingip(floatingip_id, **params)
        self.wait_resource_status(self._get_floatingip_status,
                                  ip['floatingip']['id'],
                                  'DOWN')
        return ip['floatingip']

    def _get_floatingip_status(self, resource_id):
        detail = self.network_client.show_floatingip(resource_id)
        return detail['floatingip']['status']

    def delete_network(self, network_id):
        self.network_client.delete_network(network_id)
        self.network_client.wait_for_resource_deletion('network', network_id)
        self.networks.remove(network_id)

    def delete_subnet(self, subnet_id):
        self.network_client.delete_subnet(subnet_id)
        self.network_client.wait_for_resource_deletion('subnet', subnet_id)
        self.subnets.remove(subnet_id)

    def delete_sec_group_rule(self, rule_id):
        self.network_client.delete_security_group_rule(rule_id)
        self.network_client.wait_for_resource_deletion('security_group_rule',
                                                       rule_id)
        self.security_group_rules.remove(rule_id)

    def delete_sec_group(self, sec_group_id):
        self.network_client.delete_security_group(sec_group_id)
        self.network_client.wait_for_resource_deletion('security_group',
                                                       sec_group_id)
        self.security_groups.remove(sec_group_id)

    def delete_floating_ip(self, floating_ip_id):
        self.disassociate_floating_ip(floating_ip_id)
        self.network_client.delete_floatingip(floating_ip_id)
        self.network_client.wait_for_resource_deletion('floatingip',
                                                       floating_ip_id)
        self.floating_ips.remove(floating_ip_id)

    def delete_router(self, router_id):
        # detach all the subnet
        for subnet in self.subnets:
            try:
                self.router_remove_subnet(router_id, subnet)
            except Exception as e:
                LOG.warning(_LW('detach subnet from router %(id)s failed, '
                                '%(msg)s. Traceback: %(tb)s'),
                            {'id': subnet,
                             'msg': e,
                             'tb': traceback.format_exc()})

        # remove the gateway
        self.router_unset_external_gw(router_id)

        self.network_client.delete_router(router_id)
        self.network_client.wait_for_resource_deletion('router', router_id)
        self.routers.remove(router_id)

    def delete_all(self):
        for firewall in self.firewalls:
            try:
                self.remove_firewall_all_routers(firewall)
                self.delete_firewall(firewall)
            except Exception as e:
                LOG.warning(_LW('Delete firewall %(id)s failed, %(msg)s.'
                                'Traceback: %(tb)s'),
                            {'id': firewall,
                             'msg': e,
                             'tb': traceback.format_exc()})
        for firewall_policy in self.firewall_policies:
            for firewall_rule in self.firewall_rules:
                try:
                    self.remove_firewall_policy_rule(firewall_policy,
                                                     firewall_rule)
                except Exception:
                    # the rule may not belong to policy
                    pass
        for firewall_policy in self.firewall_policies:
            try:
                self.delete_firewall_policy(firewall_policy)
            except Exception as e:
                LOG.warning(_LW('Delete firewall policy %(id)s failed, '
                                '%(msg)s. Traceback: %(tb)s'),
                            {'id': firewall_policy,
                             'msg': e,
                             'tb': traceback.format_exc()})
        for firewall_rule in self.firewall_rules:
            try:
                self.delete_firewall_rule(firewall_rule)
            except Exception as e:
                LOG.warning(_LW('Delete firewall rule %(id)s failed, '
                                '%(msg)s. Traceback: %(tb)s'),
                            {'id': firewall_rule,
                             'msg': e,
                             'tb': traceback.format_exc()})
        for ip in self.floating_ips:
            try:
                self.delete_floating_ip(ip)
            except Exception as e:
                LOG.warning(_LW('Delete floatingip %(id)s failed, %(msg)s.'
                                'Traceback: %(tb)s'),
                            {'id': ip,
                             'msg': e,
                             'tb': traceback.format_exc()})
        for sec_rule in self.security_group_rules:
            try:
                self.delete_sec_group_rule(sec_rule)
            except Exception as e:
                LOG.warning(_LW('Delete sec_grp_rule %(id)s failed, %(msg)s.'
                                'Traceback: %(tb)s'),
                            {'id': sec_rule,
                             'msg': e,
                             'tb': traceback.format_exc()})
        for sec in self.security_groups:
            try:
                self.delete_sec_group(sec)
            except Exception as e:
                LOG.warning(_LW('Delete sec_grp %(id)s failed, %(msg)s.'
                                'Traceback: %(tb)s'),
                            {'id': sec,
                             'msg': e,
                             'tb': traceback.format_exc()})
        for router in self.routers:
            try:
                self.delete_router(router)
            except Exception as e:
                LOG.warning(_LW('Delete router %(id)s failed, %(msg)s.'
                                'Traceback: %(tb)s'),
                            {'id': router,
                             'msg': e,
                             'tb': traceback.format_exc()})
        for subnet in self.subnets:
            try:
                self.delete_subnet(subnet)
            except Exception as e:
                LOG.warning(_LW('Delete subnet %(id)s failed, %(msg)s.'
                                'Traceback: %(tb)s'),
                            {'id': subnet,
                             'msg': e,
                             'tb': traceback.format_exc()})
        for net in self.networks:
            try:
                self.delete_network(net)
            except Exception as e:
                LOG.warning(_LW('Delete network %(id)s failed, %(msg)s.'
                                'Traceback: %(tb)s'),
                            {'id': net,
                             'msg': e,
                             'tb': traceback.format_exc()})
        if not self._check_delete_list():
            raise exceptions.DeleteException

    def _check_delete_list(self):
        flag = True
        if self.firewalls:
            LOG.warning(_LW('Delete firewalls not clear, '
                            'remaining %s'), self.firewalls)
            flag = False
        if self.firewall_policies:
            LOG.warning(_LW('Delete firewall_policies not clear, '
                            'remaining %s'), self.firewall_policies)
            flag = False
        if self.firewall_rules:
            LOG.warning(_LW('Delete firewall_rules not clear, '
                            'remaining %s'), self.firewall_rules)
            flag = False
        if self.floating_ips:
            LOG.warning(_LW('Delete floating_ips not clear, '
                            'remaining %s'), self.floating_ips)
            flag = False
        if self.security_group_rules:
            LOG.warning(_LW('Delete security_group_rules not clear, '
                            'remaining %s'), self.security_group_rules)
            flag = False
        if self.security_groups:
            LOG.warning(_LW('Delete security_groups not clear, '
                            'remaining %s'), self.security_groups)
            flag = False
        if self.routers:
            LOG.warning(_LW('Delete routers not clear, '
                            'remaining %s'), self.routers)
            flag = False
        if self.subnets:
            LOG.warning(_LW('Delete subnets not clear, '
                            'remaining %s'), self.subnets)
            flag = False
        if self.networks:
            LOG.warning(_LW('Delete networks not clear, '
                            'remaining %s'), self.networks)
            flag = False
        return flag

    def wait_resource_status(self, method, resource_id, status, interval=None,
                             timeout=None):
        if not interval:
            interval = CONF.network.build_interval
        if not timeout:
            timeout = CONF.network.build_timeout
        while timeout >= 0:
            timeout -= interval
            current_status = method(resource_id)
            if current_status == status:
                return
            time.sleep(interval)
        raise exceptions.TimeoutException

    def test_ping_floatingip(self, ip_address, interval=None, timeout=None):
        if not interval:
            interval = CONF.network.build_interval
        if not timeout:
            timeout = CONF.network.build_timeout
        while timeout >= 0:
            timeout -= interval
            if self._ping_floating_ip(ip_address):
                return
            time.sleep(interval)
        raise exceptions.TimeoutException

    def _ping_floating_ip(self, ip_address):
        LOG.warning(_LW('During basic network test, '
                        'try to ping %(id)s, %(ip)s'),
                    {'id': self.floating_ips, 'ip': ip_address})
        p = None
        try:
            cmd = "ping -c %s %s" % ("4", ip_address)
            p = subprocess.Popen(
                cmd, shell=True, stdout=subprocess.PIPE,
                stderr=subprocess.STDOUT)
            for line in p.stdout.readlines():
                LOG.warning(_LW('Run cmd "%(cmd)s", get result "%(res)s".'),
                            {'cmd': cmd, 'res': str(line)})
                if ', 0% packet loss' in line:
                    return True
            return False
        finally:
            if p:
                p.kill()
