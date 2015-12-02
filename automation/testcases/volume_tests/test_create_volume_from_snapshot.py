# !/usr/bin/python
import traceback

from automation.common import exceptions
from automation import config
from automation.i18n import _LE
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.volume_tests.base import Base
from testcases.volume_tests.base import BaseVolumeTest

CONF = config.CONF
LOG = logging.getLogger(__name__)


class CreateVolumeFromSnapshotTest(BaseVolumeTest):

    def __init__(self):
        try:
            self.volume = Base()
        except Exception as e:
            LOG.warning(_LW('Init test env failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
            raise exceptions.InitFailed

    def test_create_volume_from_image(self):
        try:
            volume_created = \
                self.volume.create_volume()
        except Exception as e:
            LOG.warning(_LW('Create volume before snapshot failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            self.volume.delete_all()
            raise exceptions.VolumeBuildErrorException()

        try:
            kwargs = dict(force=True)
            snapshot = self.volume.create_snapshot(volume_created['id'],
                                                   **kwargs)
        except Exception as e:
            LOG.warning(_LE('create volume snapshot failed, msg: '
                            '%(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            self.volume.delete_all()
            raise exceptions.SnapshotBuildErrorException()

        try:
            volume_created = self.volume.\
                create_volume(snapshot_id=snapshot.get('id'))
            status = self.volume._get_volume_status(volume_created['id'])
            if status != "available":
                raise exceptions.VolumeBuildErrorException()
        except Exception as e:
            LOG.warning(_LW('Create volume from snapshot failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise e
        finally:
            self.volume.delete_all()

if __name__ == '__main__':
    vt = CreateVolumeFromSnapshotTest()
    vt.test_create_volume_from_image()
