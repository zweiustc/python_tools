# Copyright 2012 OpenStack Foundation
# All Rights Reserved.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.

import copy
import time
import traceback
import uuid

from automation.clients import Manager
from automation.common import exceptions
from automation.common import fixed_network
from automation.common.utils import data_utils
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging


import testcases.test

CONF = config.CONF

LOG = logging.getLogger(__name__)


class BaseVolumeTest(testcases.test.BaseTestCase):
    """Base test case class for all Cinder API tests."""

    _api_version = 2
    credentials = ['primary']

    @classmethod
    def skip_checks(cls):
        super(BaseVolumeTest, cls).skip_checks()

        if not CONF.service_available.cinder:
            skip_msg = ("%s skipped as Cinder is not available" % cls.__name__)
            raise cls.skipException(skip_msg)
        if cls._api_version == 1:
            if not CONF.volume_feature_enabled.api_v1:
                msg = "Volume API v1 is disabled"
                raise cls.skipException(msg)
        elif cls._api_version == 2:
            if not CONF.volume_feature_enabled.api_v2:
                msg = "Volume API v2 is disabled"
                raise cls.skipException(msg)
        else:
            msg = ("Invalid Cinder API version (%s)" % cls._api_version)
            raise exceptions.InvalidConfiguration(message=msg)

    @classmethod
    def setup_credentials(cls):
        cls.set_network_resources()
        super(BaseVolumeTest, cls).setup_credentials()

    @classmethod
    def setup_clients(cls):
        super(BaseVolumeTest, cls).setup_clients()
        cls.servers_client = cls.os.servers_client
        cls.networks_client = cls.os.networks_client
        cls.images_client = cls.os.images_client

        if cls._api_version == 1:
            cls.snapshots_client = cls.os.snapshots_client
            cls.volumes_client = cls.os.volumes_client
            cls.backups_client = cls.os.backups_client
            cls.volume_services_client = cls.os.volume_services_client
            cls.volumes_extension_client = cls.os.volumes_extension_client
            cls.availability_zone_client = (
                cls.os.volume_availability_zone_client)
        else:
            cls.snapshots_client = cls.os.snapshots_v2_client
            cls.volumes_client = cls.os.volumes_v2_client
            cls.volumes_extension_client = cls.os.volumes_v2_extension_client
            cls.availability_zone_client = (
                cls.os.volume_v2_availability_zone_client)

    @classmethod
    def resource_setup(cls):
        super(BaseVolumeTest, cls).resource_setup()

        cls.snapshots = []
        cls.volumes = []
        cls.image_ref = CONF.compute.image_ref
        cls.flavor_ref = CONF.compute.flavor_ref
        cls.build_interval = CONF.volume.build_interval
        cls.build_timeout = CONF.volume.build_timeout

        if cls._api_version == 1:
            # Special fields and resp code for cinder v1
            cls.special_fields = {'name_field': 'display_name',
                                  'descrip_field': 'display_description'}
        else:
            # Special fields and resp code for cinder v2
            cls.special_fields = {'name_field': 'name',
                                  'descrip_field': 'description'}

    @classmethod
    def resource_cleanup(cls):
        cls.clear_snapshots()
        cls.clear_volumes()
        super(BaseVolumeTest, cls).resource_cleanup()

    @classmethod
    def create_volume(cls, size=None, **kwargs):
        """Wrapper utility that returns a test volume."""
        name = data_utils.rand_name('Volume')

        name_field = cls.special_fields['name_field']

        kwargs[name_field] = name
        volume = cls.volumes_client.create_volume(size, **kwargs)

        cls.volumes.append(volume)
        cls.volumes_client.wait_for_volume_status(volume['id'], 'available')
        return volume

    @classmethod
    def create_snapshot(cls, volume_id=1, **kwargs):
        """Wrapper utility that returns a test snapshot."""
        snapshot = cls.snapshots_client.create_snapshot(volume_id,
                                                        **kwargs)
        cls.snapshots.append(snapshot)
        cls.snapshots_client.wait_for_snapshot_status(snapshot['id'],
                                                      'available')
        return snapshot

    # NOTE(afazekas): these create_* and clean_* could be defined
    # only in a single location in the source, and could be more general.

    @classmethod
    def delete_all(self):
        try:
            self.resource_cleanup()
        except Exception as e:
            LOG.warning(_LW('Delete snapshots and volumes failed, %(msg)s.'
                            'Traceback: %(tb)s'),
                        {'msg': e,
                         'tb': traceback.format_exc()})

    @classmethod
    def clear_volumes(cls):
        for volume in cls.volumes:
            try:
                cls.volumes_client.delete_volume(volume['id'])
            except Exception:
                pass

        for volume in cls.volumes:
            try:
                cls.volumes_client.wait_for_resource_deletion(volume['id'])
            except Exception:
                pass

    @classmethod
    def clear_snapshots(cls):
        for snapshot in cls.snapshots:
            try:
                cls.snapshots_client.delete_snapshot(snapshot['id'])
            except Exception:
                pass

        for snapshot in cls.snapshots:
            try:
                cls.snapshots_client.wait_for_resource_deletion(snapshot['id'])
            except Exception:
                pass

    @classmethod
    def create_server(cls, name, **kwargs):
        network = cls.get_tenant_network()
        network_kwargs = fixed_network.set_networks_kwarg(network, kwargs)
        return cls.servers_client.create_server(name,
                                                cls.image_ref,
                                                cls.flavor_ref,
                                                **network_kwargs)


class BaseVolumeAdminTest(BaseVolumeTest):
    """Base test case class for all Volume Admin API tests."""

    credentials = ['primary', 'admin']

    @classmethod
    def setup_clients(cls):
        super(BaseVolumeAdminTest, cls).setup_clients()

        if cls._api_version == 1:
            cls.volume_qos_client = cls.os_adm.volume_qos_client
            cls.admin_volume_services_client = \
                cls.os_adm.volume_services_client
            cls.volume_types_client = cls.os_adm.volume_types_client
            cls.admin_volume_client = cls.os_adm.volumes_client
            cls.hosts_client = cls.os_adm.volume_hosts_client
            cls.admin_snapshots_client = cls.os_adm.snapshots_client
            cls.backups_adm_client = cls.os_adm.backups_client
            cls.quotas_client = cls.os_adm.volume_quotas_client
        elif cls._api_version == 2:
            cls.volume_qos_client = cls.os_adm.volume_qos_v2_client
            cls.admin_volume_services_client = \
                cls.os_adm.volume_services_v2_client
            cls.volume_types_client = cls.os_adm.volume_types_v2_client
            cls.admin_volume_client = cls.os_adm.volumes_v2_client
            cls.hosts_client = cls.os_adm.volume_hosts_v2_client
            cls.admin_snapshots_client = cls.os_adm.snapshots_v2_client
            cls.backups_adm_client = cls.os_adm.backups_v2_client
            cls.quotas_client = cls.os_adm.volume_quotas_v2_client

    @classmethod
    def resource_setup(cls):
        super(BaseVolumeAdminTest, cls).resource_setup()

        cls.qos_specs = []

    @classmethod
    def resource_cleanup(cls):
        cls.clear_qos_specs()
        super(BaseVolumeAdminTest, cls).resource_cleanup()

    @classmethod
    def create_test_qos_specs(cls, name=None, consumer=None, **kwargs):
        """create a test Qos-Specs."""
        name = name or data_utils.rand_name(cls.__name__ + '-QoS')
        consumer = consumer or 'front-end'
        qos_specs = cls.volume_qos_client.create_qos(name, consumer,
                                                     **kwargs)
        cls.qos_specs.append(qos_specs['id'])
        return qos_specs

    @classmethod
    def clear_qos_specs(cls):
        for qos_id in cls.qos_specs:
            try:
                cls.volume_qos_client.delete_qos(qos_id)
            except exceptions.NotFound:
                # The qos_specs may have already been deleted which is OK.
                pass

        for qos_id in cls.qos_specs:
            try:
                cls.volume_qos_client.wait_for_resource_deletion(qos_id)
            except exceptions.NotFound:
                # The qos_specs may have already been deleted which is OK.
                pass


class Base(object):

    def __init__(self, volume_size=None, volume_backend=None):
        self.manager = Manager()
        self.volumes = list()
        self.snapshots = list()
        self.volume_types = list()
        self.qos_specs = list()
        self.volume_size = volume_size or CONF.volume.volume_size
        self.volume_backend = volume_backend or CONF.volume.backend1_name

    def create_volume(self, **kwargs):
        volume_name = str(uuid.uuid4())
        kwargs['name'] = volume_name

        volume = self.manager.volumes_v2_client.create_volume(self.volume_size,
                                                              **kwargs)
        self.volumes.append(volume['id'])
        try:
            self.check_volume_status_interval(volume['id'], 'available')
        except exceptions.ServerActionSucceed:
            return volume
        return volume

    def show_volume(self, volume_id):
        volume = self.manager.volumes_v2_client.show_volume(volume_id)
        return volume

    def extend_volume(self, volume_id, extend_size=CONF.volume.extend_size):
        self.manager.volumes_v2_client.extend_volume(
            volume_id, extend_size)
        try:
            self.check_volume_status_interval(volume_id, 'available')
        except exceptions.ServerActionSucceed:
            return True
        return True

    def update_volume(self, volume_id, **kwargs):
        self.manager.volumes_v2_client.update_volume(volume_id, **kwargs)
        try:
            self.check_volume_status_interval(volume_id, 'available')
        except exceptions.ServerActionSucceed:
            return True
        return True

    def rollback_volume(self, volume_id, snapshot_id):
        self.manager.volumes_v2_client.rollback_volume(volume_id,
                                                       snapshot_id)
        try:
            self.check_volume_status_interval(volume_id, 'available')
            self.check_snapshot_status_interval(snapshot_id, 'available')
        except exceptions.ServerActionSucceed:
            return True
        return True

    def delete_volume(self, volume_id):
        self.manager.volumes_v2_client.delete_volume(volume_id)

    def associate_volumetype_with_qos(self, qos_id, volumetype_id):
        self.manager.volume_qos_v2_client.associate_qos(qos_id, volumetype_id)

    def disassociate_volumetype_with_qos(self, qos_id, volumetype_id):
        self.manager.volume_qos_v2_client.disassociate_qos(qos_id,
                                                           volumetype_id)

    def disassociate_all(self, qos_id):
        self.manager.volume_qos_v2_client.disassociate_all_qos(qos_id)

    def set_qos_key(self, qos_id, **kwargs):
        self.manager.volume_qos_v2_client.set_qos_key(qos_id, **kwargs)

    def unset_qos_key(self, qos_id, keys):
        self.manager.volume_qos_v2_client.unset_qos_key(qos_id, keys)

    def show_volume_type(self, volume_type_id):
        volume_type_details = \
            self.manager.volume_types_v2_client.show_volume_type(
                volume_type_id)
        return volume_type_details

    def show_qos_type(self, qos_id):
        qos_type_details = self.manager.volume_qos_v2_client.show_qos(qos_id)
        return qos_type_details

    def retype_volume(self, volume_id, volume_type, **kwargs):
        self.manager.volumes_v2_client.retype_volume(volume_id, volume_type,
                                                     **kwargs)

    def create_test_volume_type(self, name=None, **kwargs):
        """create a test Volume-Types."""
        volume_type_rand_prefix = uuid.uuid4().hex
        # name = name or data_utils.rand_name(
        #    volume_type_rand_prefix + '-volumetype')
        name = name or (volume_type_rand_prefix + '-volumetype')
        LOG.info("volume_type name prefix is %s", volume_type_rand_prefix)
        volume_type = \
            self.manager.volume_types_v2_client.create_volume_type(name,
                                                                   **kwargs)
        self.volume_types.append(volume_type)
        return volume_type

    def create_test_qos_specs(self, name=None, consumer=None, **kwargs):
        """create a test Qos-Specs."""
        qos_type_rand_prefix = uuid.uuid4().hex
        name = name or data_utils.rand_name(qos_type_rand_prefix + '-QoS')
        consumer = consumer or 'front-end'
        qos_specs = self.manager.volume_qos_v2_client.create_qos(name,
                                                                 consumer,
                                                                 **kwargs)
        self.qos_specs.append(qos_specs)
        return qos_specs

    def clear_volume_types(self):
        volume_types_copy = copy.deepcopy(self.volume_types)
        for volume_type in volume_types_copy:
            try:
                self.manager.volume_types_v2_client.delete_volume_type(
                    volume_type['id'])
                self.volume_types.remove(volume_type)
            except exceptions.NotFound:
                # The volume_types may have already been deleted which is OK.
                pass
        '''
        for volume_type in self.volume_types:
            try:
                self.manager.volume_types_v2_client.wait_for_resource_deletion(
                    volume_type)
            except exceptions.NotFound:
                # The volume_types may have already been deleted which is OK.
                pass
        '''

    def clear_qos_specs(self):
        qos_specs_copy = copy.deepcopy(self.qos_specs)
        for qos in qos_specs_copy:
            try:
                self.manager.volume_qos_v2_client.delete_qos(qos['id'])
                self.qos_specs.remove(qos)
            except exceptions.NotFound:
                # The qos_specs may have already been deleted which is OK.
                pass

        for qos in self.qos_specs:
            try:
                self.manager.volume_qos_v2_client.wait_for_resource_deletion(
                    qos['id'])
            except exceptions.NotFound:
                # The qos_specs may have already been deleted which is OK.
                pass

    def delete_all(self):
        is_successfully_deleted = True
        for snapshot in self.snapshots:
            try:
                self.delete_snapshot(snapshot)
            except Exception as e:
                LOG.warning(_LW('Delete server %(server_id)s failed, %(msg)s.'
                                'Traceback: %(tb)s'),
                            {'server_id': snapshot,
                             'msg': e,
                             'tb': traceback.format_exc()})

        for volume in self.volumes:
            try:
                self.delete_volume(volume)
            except Exception as e:
                LOG.warning(_LW('Delete server %(server_id)s failed, %(msg)s.'
                                'Traceback: %(tb)s'),
                            {'server_id': volume,
                             'msg': e,
                             'tb': traceback.format_exc()})

        check_interval = CONF.volume.check_delete_interval
        check_timeout = CONF.volume.check_delete_timeout
        while self.volumes and check_timeout >= 0:
            check_timeout -= check_interval
            for volume in self.volumes:
                try:
                    self._get_volume_status(volume)
                except exceptions.NotFound:
                    self.volumes.remove(volume)
        if self.volumes:
            LOG.warning(_LW('Delete volume timeout, remaining volume: %(v)s'),
                        {'v': self.volumes})
            self.volumes = []
            LOG.warning(_LW('During volumes delete failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            is_successfully_deleted = False

        try:
            self.clear_volume_types()
        except Exception as e:
            LOG.warning(_LW('During volume type delete failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            is_successfully_deleted = False

        try:
            self.clear_qos_specs()
        except Exception as e:
            LOG.warning(_LW('During qos type delete failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            is_successfully_deleted = False

        if not is_successfully_deleted:
            raise exceptions.DeleteException

    def _get_volume_status(self, volume_id):
        volume_detail = self.manager.volumes_v2_client.show_volume(volume_id)
        return volume_detail.get('status')

    def check_volume_status_interval(self, volume_id, status='available',
                                     interval=CONF.volume.build_interval,
                                     timeout=CONF.volume.build_timeout):
        check_build_interval = interval
        check_timeout = timeout
        while check_timeout >= 0:
            server_status = self._get_volume_status(volume_id)
            if server_status == status:
                return
            elif server_status == "error":
                LOG.warning(_LW('Volume %(id)s changed into error state '
                                'while waiting to change into %(status)s.'),
                            {'id': volume_id, 'status': status})
                raise exceptions.ServerError
            check_timeout -= check_build_interval
            time.sleep(check_build_interval)
        LOG.warning(_LW('Volume %(id)s wait time out,'
                        'while waiting to change into %(status)s.'),
                    {'id': volume_id, 'status': status})
        raise exceptions.ServerTimeOut

    def create_snapshot(self, volume_id=1, **kwargs):
        """Wrapper utility that returns a test snapshot."""
        snapshot = \
            self.manager.snapshots_v2_client.create_snapshot(volume_id,
                                                             **kwargs)
        self.snapshots.append(snapshot['id'])
        try:
            self.check_snapshot_status_interval(snapshot['id'], 'available')
            return snapshot
        except exceptions.ServerActionSucceed:
            return snapshot

    def delete_snapshot(self, snapshot_id):
        self.manager.snapshots_v2_client.delete_snapshot(snapshot_id)
        self.snapshots.remove(snapshot_id)
        try:
            # need to wait until the snapshot has been deleted
            self.check_snapshot_status_interval(snapshot_id, None)
        except Exception:
            # the snapshot may have been deleted
            pass

    def _get_snapshot_status(self, snapshot_id):
        snapshot_detail = \
            self.manager.snapshots_v2_client.show_snapshot(snapshot_id)
        return snapshot_detail.get('status')

    def check_snapshot_status_interval(self, snapshot_id, status='available',
                                       interval=CONF.volume.build_interval,
                                       timeout=CONF.volume.build_timeout):
        check_build_interval = interval
        check_timeout = timeout
        while check_timeout >= 0:
            server_status = self._get_snapshot_status(snapshot_id)
            if server_status == status:
                return
            elif server_status == "error":
                LOG.warning(_LW('Volume %(id)s changed into error state '
                                'while waiting to change into %(status)s.'),
                            {'id': snapshot_id, 'status': status})
                raise exceptions.ServerError
            check_timeout -= check_build_interval
            time.sleep(check_build_interval)
        LOG.warning(_LW('Volume %(id)s wait time out,'
                        'while waiting to change into %(status)s.'),
                    {'id': snapshot_id, 'status': status})
        raise exceptions.ServerTimeOut
