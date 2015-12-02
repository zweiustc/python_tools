# !/usr/bin/python
import traceback

from automation.common import exceptions
from automation import config
from automation.i18n import _LE
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.volume_tests.base import Base

CONF = config.CONF
LOG = logging.getLogger(__name__)


class RollbackVolumeSnaphotTest(Base):

    def test_rollback_volume_snapshot(self):
        try:
            volume = Base()
        except Exception as e:
            LOG.warning(_LW('Init test env failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise exceptions.InitFailed

        try:
            volume_created = volume.create_volume()
        except Exception as e:
            LOG.warning(_LW('Create volume before snapshot failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise exceptions.VolumeBuildErrorException()

        try:
            snapshot = volume.create_snapshot(volume_created['id'])
        except Exception as e:
            LOG.warning(_LE('create volume snapshot failed, msg: '
                            '%(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise exceptions.SnapshotBuildErrorException()

        try:
            # rollback the volume with snapshot
            volume.rollback_volume(volume_created['id'],
                                   snapshot['id'])
        except Exception as e:
            LOG.warning(_LW('Create volume before snapshot failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise exceptions.VolumeRollbackErrorException()
        finally:
            volume.delete_all()

if __name__ == "__main__":
    volume_test = RollbackVolumeSnaphotTest()
    volume_test.test_rollback_volume_snapshot()
