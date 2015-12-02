# !/usr/bin/python
import time
import traceback
import uuid

from automation.clients import Manager
from automation.common import exceptions
from automation.common.utils import data_utils
from automation.common import waiters
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging

from testcases.base_ssh import BaseSSH
from testcases.compute_tests.base_monitor import MonitorBase

CONF = config.CONF
LOG = logging.getLogger(__name__)


class Base(object):

    def __init__(self, flavor_ref=None, image_ref=None, network_id=None,
                 security_group_id=None, server_name=None):
        self.manager = Manager()
        self.servers = list()
        self.images = list()
        self.image_ref = image_ref or self._get_image_id()
        self.server_name = server_name if server_name else str(uuid.uuid4())
        self.flavor_ref = flavor_ref or self._get_flavor_id()
        self.network_id = network_id or self._get_network_id()
        self.network_ref = [{"uuid": self.network_id}]
        self.security_group_id = security_group_id
        self.monitor = MonitorBase()

    def create_server(self, name=None, image_id=None,
                      flavor_id=None, network_id=None,
                      sec_grp_id=None):
        server_name = name if name else self.server_name
        image_ref = image_id if image_id else self.image_ref
        flavor_ref = flavor_id if flavor_id else self.flavor_ref
        network_ref = \
            [{"uuid": network_id}] if network_id else self.network_ref
        params = dict()
        security_group_id = \
            sec_grp_id if sec_grp_id else self.security_group_id
        if security_group_id:
            params.update({"security_groups":
                           [{"name": security_group_id}]})
        server = self.manager.servers_client.create_server(
            server_name, image_ref, flavor_ref, networks=network_ref, **params)
        self.servers.append(server['id'])
        try:
            self._check_server_status_interval(server['id'], 'ACTIVE')
            self.monitor.create_monitor(server['id'])
        except exceptions.ServerActionSucceed:
            return server
        return server

    def attach_volume(self, server_id, volume_id):
        self.manager.servers_client.attach_volume(server_id, volume_id)
        try:
            self._check_server_volume_interval(server_id, volume_id)
        except exceptions.ServerActionSucceed:
            return True
        return True

    def detach_volume(self, server_id, volume_id):
        self.manager.servers_client.detach_volume(server_id, volume_id)

    def delete_all(self):
        for server in self.servers:
            try:
                self.delete_server(server)
            except Exception as e:
                LOG.warning(_LW('Delete server %(server_id)s failed, %(msg)s.'
                                'Traceback: %(tb)s'),
                            {'server_id': server,
                             'msg': e,
                             'tb': traceback.format_exc()})
        check_interval = CONF.compute.check_delete_interval
        check_timeout = CONF.compute.check_delete_timeout
        while self.servers and check_timeout >= 0:
            check_timeout -= check_interval
            for server in self.servers:
                try:
                    self._get_server_status(server)
                except exceptions.NotFound:
                    self.servers.remove(server)
            time.sleep(check_interval)
        # clear all the images
        for image in self.images:
            try:
                self.delete_image(image)
            except exceptions as e:
                LOG.warning(_LW('Delete image %(image_id)s failed, %(msg)s.'
                                'Traceback: %(tb)s'),
                            {'image_id': image,
                             'msg': e,
                             'tb': traceback.format_exc()})
        check_interval = CONF.compute.check_delete_interval
        check_timeout = CONF.compute.check_delete_timeout
        while self.images and check_timeout >= 0:
            check_timeout -= check_interval
            time.sleep(check_interval)
            for image in self.images:
                try:
                    status = self._get_image_status(image)
                    if status == "DELETED":
                        self.images.remove(image)
                except exceptions as e:
                    LOG.warning(_LW('Check image %(image_id)s status failed, '
                                    '%(msg)s. Traceback: %(tb)s'),
                                {'image_id': image,
                                 'msg': e,
                                 'tb': traceback.format_exc()})
        if self.servers:
            LOG.warning(_LW('Delete server timeout, remaining server: %(v)s'),
                        {'v': self.servers})
        if self.images:
            LOG.warning(_LW('Clear image(s) timeout, remaining images: %(v)s'),
                        {'v': self.images})
        if self.servers:
            self.servers = []
            raise exceptions.ServerDeleteException
        if self.images:
            self.images = []
            raise exceptions.ImageKilledException

    def delete_server(self, server_id):
        self.manager.servers_client.delete_server(server_id)

    def show_server(self, server_id):
        return self.manager.servers_client.show_server(server_id)

    def power_off(self, server_id):
        self.manager.servers_client.stop(server_id)
        try:
            self._check_server_status_interval(server_id, 'SHUTOFF')
        except exceptions.ServerActionSucceed:
            return True
        return True

    def power_on(self, server_id):
        self.manager.servers_client.start(server_id)
        try:
            self._check_server_status_interval(server_id, 'ACTIVE')
        except exceptions.ServerActionSucceed:
            return True
        return True

    def reboot(self, server_id, reboot_type):
        self.manager.servers_client.reboot(server_id, reboot_type)
        try:
            self._check_server_status_interval(server_id, 'ACTIVE')
        except exceptions.ServerActionSucceed:
            return True
        return True

    def resize(self, server_id, flavor_name):
        flavor_id = self._get_flavor_id(flavor_name)
        self.manager.servers_client.resize(server_id, flavor_id)
        try:

            self._check_server_status_interval(server_id, "VERIFY_RESIZE")
            self.manager.servers_client.confirm_resize(server_id)
        except exceptions.ServerActionSucceed:
            pass

        try:
            self._check_server_status_interval(server_id, "ACTIVE")
        except exceptions.ServerActionSucceed:
            return True
        return True

    def set_server_network_qos(self, server_id, current_qos_id, new_qos_id):
        self.manager.servers_client.set_server_network_qos(
            server_id, current_qos_id, new_qos_id)
        try:
            self._check_server_status_interval(server_id, "ACTIVE")
        except exceptions.ServerActionSucceed:
            return True

    def get_server_current_qos(self, server_id):
        current_qos = self.manager.servers_client.get_server_network_qos(
            server_id)
        current_qos_id = current_qos.get('network_vif_qos_id')
        if not current_qos_id:
            current_qos_id = "10"
        return current_qos_id

    def get_qos_types(self):
        return self.manager.servers_client.get_network_qos_types()

    def _get_image_id(self, name=CONF.compute.image_name):
        images_resp = self.manager.image_client.list_images()
        images = images_resp.get('images', [])
        for image in images:
            if image['name'] == name:
                return image['id']
        raise exceptions.ImageNotFound

    def _get_flavor_id(self, name=CONF.compute.flavor_name):
        flavors = self.manager.flavors_client.list_flavors()
        for flavor in flavors:
            if flavor['name'] == name:
                return flavor['id']
        raise exceptions.FlavorNotFound

    def _get_network_id(self, network_name=CONF.compute.fixed_network_name):
        networks_resp = self.manager.network_client.list_networks()
        networks = networks_resp.get('networks', [])
        for network in networks:
            if network['name'] == network_name:
                return network['id']
        raise exceptions.NetworkNotFound

    def get_server_host(self, server_id):
        server_detail = self.manager.servers_client.show_server(server_id)
        return server_detail.get('OS-EXT-SRV-ATTR:host', None)

    def _get_server_status(self, server_id):
        server_detail = self.manager.servers_client.show_server(server_id)
        return server_detail.get('status')

    def _get_server_task_state(self, server_id):
        server_detail = self.manager.servers_client.show_server(server_id)
        status = server_detail.get('status')
        task_state = server_detail.get('OS-EXT-STS:task_state')
        return status, task_state

    def _get_server_volumes(self, server_id):
        """
        :param server_id:
        :return: list of volumes attached to the server
        """
        server_detail = self.manager.servers_client.show_server(server_id)
        volume_dict = server_detail.get('os-extended-volumes:volumes_attached')
        volume_list = list()
        for volume in volume_dict:
            volume_list.append(volume['id'])
        return volume_list

    def _check_server_status_interval(self, server_id, status='ACTIVE',
                                      interval=CONF.compute.build_interval,
                                      timeout=CONF.compute.build_timeout):
        """Check server status interval, will raise status"""
        check_build_interval = interval
        check_timeout = timeout
        while check_timeout >= 0:
            server_status = self._get_server_status(server_id)
            if server_status == status:
                return
            elif server_status == "ERROR":
                LOG.warning(_LW('Instance %(id)s changed into ERROR state '
                                'while waiting to change into %(status)s.'),
                            {'id': server_id, 'status': status})
                raise exceptions.ServerError
            check_timeout -= check_build_interval
            time.sleep(check_build_interval)
        LOG.warning(_LW('Instance %(id)s wait time out,'
                        'while waiting to change into %(status)s.'),
                    {'id': server_id, 'status': status})
        raise exceptions.ServerTimeOut

    def _check_server_task_state_interval(self, server_id, t_task_state=None,
                                          interval=CONF.compute.build_interval,
                                          timeout=CONF.compute.build_timeout):
        """Check server status interval, will raise status"""
        check_build_interval = interval
        check_timeout = timeout
        while check_timeout >= 0:
            server_status, task_state = self._get_server_task_state(server_id)
            if task_state.lower() == t_task_state:
                return
            elif server_status == "ERROR":
                LOG.warning(_LW('Instance %(id)s changed into ERROR state '
                                'while waiting task state %(status)s.'),
                            {'id': server_id, 'status': t_task_state})
                raise exceptions.ServerError
            check_timeout -= check_build_interval
            time.sleep(check_build_interval)
        LOG.warning(_LW('Instance %(id)s wait time out,'
                        'while waiting task state %(status)s.'),
                    {'id': server_id, 'status': t_task_state})
        raise exceptions.ServerTimeOut

    def _check_server_volume_interval(self, server_id, volume_id,
                                      interval=CONF.compute.build_interval,
                                      timeout=CONF.compute.build_timeout):
        """Check server status interval, will raise status"""
        check_build_interval = interval
        check_timeout = timeout
        while check_timeout >= 0:
            volumes = self._get_server_volumes(server_id)
            if volume_id in volumes:
                return
            check_timeout -= check_build_interval
            time.sleep(check_build_interval)
        LOG.warning(_LW('Instance %(id)s wait time out,'
                        'while waiting attach volume %(status)s.'),
                    {'id': server_id, 'status': volume_id})
        raise exceptions.ServerTimeOut

    def _check_server_start_status_interval(self, server_ip, password, command,
                                            interval=CONF.
                                            compute.build_interval,
                                            timeout=CONF.
                                            compute.build_timeout):
        """Check server status interval, will raise status"""
        check_build_interval = interval
        check_timeout = timeout
        while check_timeout >= 0:
            ret = BaseSSH.run_cmd(server_ip,
                                  passwd=password,
                                  cmd=command)
            if ret is not None:
                return ret
            check_timeout -= check_build_interval
            time.sleep(check_build_interval)
        raise exceptions.ServerTimeOut

    def show_server_interfaces(self, server_id):
        return self.manager.servers_client.list_interfaces(server_id)

    def _capture_image_from_server(self, server_id, **kwargs):
        """Wrapper utility that returns an image created from the server."""
        name = self.__class__.__name__ + "-image-" + str(uuid.uuid4())[:5]
        if 'name' in kwargs:
            name = kwargs.pop('name')
        image = self.manager.servers_client.capture_image(server_id, name)
        image_id = data_utils.parse_image_id(image.response['image_id'])
        self.images.append(image_id)
        show_image = None
        if 'wait_until' in kwargs:
            waiters.wait_for_image_status(self.manager.images_client,
                                          image_id, kwargs['wait_until'])
            show_image = self.manager.images_client.show_image(image_id)
        return show_image

    def delete_image(self, image_id):
        self.manager.image_client.delete_image(image_id)

    def _get_image_status(self, image_id):
        image_detail = self.manager.images_client.show_image(image_id)
        return image_detail.get('status')
