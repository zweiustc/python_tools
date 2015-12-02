# !/usr/bin/python
import traceback
import uuid

from automation.common import exceptions
from automation import config
from automation.i18n import _LI
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.volume_tests.base import Base


CONF = config.CONF
LOG = logging.getLogger(__name__)


class TestCreateVolumeWithQos(Base):

    __disk_qos_low = {"read_iops_sec": "100",
                      "write_iops_sec": "50",
                      "read_bytes_sec": "409600",
                      "write_bytes_sec": "204800"}

    __disk_qos_medium = {"read_iops_sec": "200",
                         "write_iops_sec": "100",
                         "read_bytes_sec": "819200",
                         "write_bytes_sec": "409600"}

    def test_create_volume_with_qos(self):
        try:
            volume = Base()
        except Exception as e:
            LOG.warning(_LW('Init test env failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise exceptions.InitFailed

        '''
        try:
            compute = compute_base()
        except Exception as e:
            LOG.warning(_LW('Init test env failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise exceptions.ServerFault
        '''

        try:
            volume_type = volume.create_test_volume_type()
        except Exception as e:
            LOG.warning(_LW('Create volume_type before create volume failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise
            # raise exceptions.VolumeBuildErrorException()

        try:
            volumetype_randname, fixedname = volume_type['name'].split('-')
            LOG.info(_LI("Volume_type fixedname is %s"), fixedname)
            qos_name = volumetype_randname + '-Qos'
            qos_type = volume.create_test_qos_specs(qos_name,
                                                    **self.__disk_qos_low)
        except Exception as e:
            LOG.warning(_LW('Create qos_type before create volume failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise

        try:
            volume.associate_volumetype_with_qos(qos_type['id'],
                                                 volume_type['id'])
        except Exception as e:
            LOG.warning(_LW('Associate qos_type with volume_type failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise

        try:
            volume_name = str(uuid.uuid4())
            params = {'volume_type': volume_type['id'],
                      'name': volume_name}
            single_volume = volume.create_volume(**params)
            LOG.debug("current test volume is %s", single_volume)
            # volume.check_volume_status_interval(single_volume['id'],
            #                                    'available')
        except Exception as e:
            LOG.warning(_LW('Create volume before snapshot failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise
        finally:
            volume.delete_all()

        '''
        try:
            single_vm = compute.create_server()
        except Exception as e:
            LOG.warning(_LW('Create server before failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            compute.delete_all()
            raise

        try:
            mount_point = '/dev/vdc'
            self.volume_client.attach_volume(single_volume['id'],
                                             single_vm['id'],
                                             mount_point)
            self.volume_client.wait_for_volume_status(single_volume['id'],
                                                      'in-use')
        except Exception as e:
            LOG.warning(_LW('Attach volume for server failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            compute.delete_all()
            volume.delete_all()
            raise

        try:
            result = self._check_volume_iops(single_vm['id'],
                                             single_volume['id'])
            return result
        except Exception as e:
            LOG.warning(_LW('Attach volume for server failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            compute.delete_all()
            volume.delete_all()
            raise
            # raise exceptions.VolumeBuildErrorException()

        try:
            mount_point = '/dev/vdc'
            self.volume_client.detach_volume(single_volume['id'])
            self.volume_client.wait_for_volume_status(single_volume['id'],
                                                      'available')
            compute.delete_all()
            volume.delete_all()
        except Exception as e:
            LOG.warning(_LW('Detach volume failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            compute.delete_all()
            volume.delete_all()
            raise
        '''

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
    volume_type_test = TestCreateVolumeWithQos()
    volume_type_test.test_create_volume_with_qos()
