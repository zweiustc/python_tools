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
            self.image_client = Manager().image_client_v2
        except Exception as e:
            LOG.warning(_LW('Init test env failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
            raise exceptions.InitFailed

    def test_create_volume_from_image(self):
        try:
            image_name = CONF.compute.image_name
            params = {"name": image_name}
            images = self.image_client.list_images(params)
            volume_created = self.volume.\
                create_volume(imageRef=images[0].get('id'))
            status = self.volume._get_volume_status(volume_created['id'])
            if status != "available":
                raise exceptions.VolumeBuildErrorException()
        except Exception as e:
            LOG.warning(_LW('Create volume from image failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise e
        finally:
            self.volume.delete_all()

if __name__ == '__main__':
    vt = CreateVolumeFromImageTest()
    vt.test_create_volume_from_image()
