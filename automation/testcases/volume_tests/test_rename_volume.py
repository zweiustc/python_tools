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


class TestRenameVolume(Base):

    def test_rename_volume(self):
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
            LOG.warning(_LW('Create volume failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise

        try:
            params = {'name': 'newname'}
            volume.update_volume(volume_created['id'], **params)
        except Exception as e:
            LOG.warning(_LE('Update volume name failed, msg: '
                            '%(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise
        finally:
            volume.delete_all()

if __name__ == "__main__":
    volume_test = TestRenameVolume()
    volume_test.test_rename_volume()
