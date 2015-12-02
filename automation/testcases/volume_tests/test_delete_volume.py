# !/usr/bin/python
import traceback

from automation.clients import Manager
from automation.common import exceptions
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.volume_tests.base import Base
from testcases.volume_tests.base import BaseVolumeTest

CONF = config.CONF
LOG = logging.getLogger(__name__)


class CreateVolumeFromImageTest(BaseVolumeTest):

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
            LOG.warning(_LW('Create volume failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise exceptions.VolumeBuildErrorException()

        try:
            Manager().volumes_v2_client.delete_volume(volume_created.get("id"))
        except Exception as e:
            LOG.warning(_LW('Delete volume failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise e


if __name__ == '__main__':
    vt = CreateVolumeFromImageTest()
    vt.test_create_volume_from_image()
