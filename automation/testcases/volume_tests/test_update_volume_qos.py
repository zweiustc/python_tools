# !/usr/bin/python
import traceback
import uuid

from automation.common import exceptions
from automation import config
# from automation.i18n import _LE
# from automation.i18n import _LI
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.volume_tests.base import Base


CONF = config.CONF
LOG = logging.getLogger(__name__)


class TestUpdateVolumeQos(Base):

    __disk_qos_low = {"read_iops_sec": "100",
                      "write_iops_sec": "50",
                      "read_bytes_sec": "409600",
                      "write_bytes_sec": "204800"}

    __disk_qos_medium = {"read_iops_sec": "200",
                         "write_iops_sec": "100",
                         "read_bytes_sec": "819200",
                         "write_bytes_sec": "409600"}

    __low_io_volumetype_name = 'low_volume_type'
    __medium_io_volumetype_name = 'medium_volume_type'

    def test_update_volume_qos(self):
        try:
            volume = Base()
        except Exception as e:
            LOG.warning(_LW('Init test env failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise exceptions.InitFailed

        try:
            low_volume_type = volume.create_test_volume_type(
                name=self.__low_io_volumetype_name)
            medium_volume_type = volume.create_test_volume_type(
                name=self.__medium_io_volumetype_name)
        except Exception as e:
            LOG.warning(_LW('Create volume_type before create volume failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise

        try:
            low_qos_name = self.__low_io_volumetype_name + '-Qos'
            low_qos_type = volume.create_test_qos_specs(name=low_qos_name,
                                                        **self.__disk_qos_low)
            medium_qos_name = self.__medium_io_volumetype_name + '-Qos'
            medium_qos_type = \
                volume.create_test_qos_specs(name=medium_qos_name,
                                             **self.__disk_qos_medium)

        except Exception as e:
            LOG.warning(_LW('Create qos_type before create volume failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise

        try:
            volume.associate_volumetype_with_qos(low_qos_type['id'],
                                                 low_volume_type['id'])
            volume.associate_volumetype_with_qos(medium_qos_type['id'],
                                                 medium_volume_type['id'])
        except Exception as e:
            LOG.warning(_LW('Associate qos_type with volume_type failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise

        try:
            volume_name = str(uuid.uuid4())
            params = {'volume_type': low_volume_type['id'],
                      'name': volume_name}
            single_volume = \
                volume.create_volume(**params)
        except Exception as e:
            LOG.warning(_LW('Create volume before snapshot failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise

        try:
            volume.retype_volume(single_volume['id'],
                                 medium_volume_type['id'])
            volume.check_volume_status_interval(single_volume['id'],
                                                'available')
        except Exception as e:
            LOG.warning(_LW('Retype volume failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise

        try:
            retyped_volume = volume.show_volume(single_volume['id'])
            current_volume_type = retyped_volume['volume_type']
            if current_volume_type == medium_volume_type['name']:
                LOG.info("The volume has been retype successfully.")
            else:
                LOG.warning(_LW('Retype volume failed'))
                volume.delete_all()
        except Exception as e:
            LOG.warning(_LW('Retype volume result check failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise
        finally:
            volume.delete_all()

    '''
    def _check_volume_iops(self, instance, volume_id):
        server_detail = instance.show_server(instance['id'])
        domain_name = server_detail.get("OS-EXT-SRV-ATTR:instance_name")
        host = server_detail.get("OS-EXT-SRV-ATTR:host")

        cmd = "fio -filename=/dev/vdc -direct=1 -iodepth 64 -thread \
              -rw=read -ioengine=libaio -bs=4k -size=1G -numjobs=1 \
              -runtime=1000 -group_reporting -name=mytest"
        ret = BaseSSH.run_cmd(host, None, cmd)
        if not ret:
            raise
    '''

if __name__ == "__main__":
    volume_type_test = TestUpdateVolumeQos()
    volume_type_test.test_update_volume_qos()
