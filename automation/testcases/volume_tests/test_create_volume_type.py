# !/usr/bin/python
import traceback

from automation.common import exceptions
from automation import config
from automation.i18n import _LI
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.volume_tests.base import Base


CONF = config.CONF
LOG = logging.getLogger(__name__)


class TestVolumeType(Base):

    def test_create_volume_type(self):
        try:
            volume = Base()
        except Exception as e:
            LOG.warning(_LW('Init test env failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise exceptions.InitFailed

        try:
            volume_type_created = \
                volume.create_test_volume_type()
            LOG.debug(_LI("The new volume_type is %s"), volume_type_created)
        except Exception as e:
            LOG.warning(_LW('Create volume type failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise
        finally:
            volume.delete_all()


if __name__ == "__main__":
    volume_type_test = TestVolumeType()
    volume_type_test.test_create_volume_type()
