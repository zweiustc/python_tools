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


class DeleteVolumeSnaphotTest(Base):

    def test_delete_volume_snapshot(self):
        try:
            volume = Base()
        except Exception as e:
            LOG.warning(_LW('Init test env failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise exceptions.InitFailed

        try:
            volume_created = \
                volume.create_volume()
        except Exception as e:
            LOG.warning(_LW('Create volume before snapshot failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise exceptions.VolumeBuildErrorException()

        try:
            kwargs = dict(force=True)
            snapshot = \
                volume.create_snapshot(volume_created['id'],
                                       **kwargs)
        except Exception as e:
            LOG.warning(_LE('create volume snapshot failed, msg: '
                            '%(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise exceptions.SnapshotBuildErrorException()

        try:
            volume.delete_snapshot(snapshot['id'])
        except Exception as e:
            LOG.warning(_LE('create volume snapshot failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise exceptions.SnapshotBuildErrorException()
        finally:
            volume.delete_all()


if __name__ == "__main__":
    volume_test = DeleteVolumeSnaphotTest()
    volume_test.test_delete_volume_snapshot()
