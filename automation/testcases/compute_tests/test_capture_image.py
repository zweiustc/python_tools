# !/usr/bin/python
import base_compute as base
import traceback

from automation.common import exceptions
from automation.common.utils import data_utils
from automation.common import waiters
from automation import config
from automation.i18n import _LW

from oslo_log import log as logging


CONF = config.CONF
LOG = logging.getLogger(__name__)


class CaptureImageTest(base.Base):

    def test_capture_image(self):
        instance = None
        captured_image = None
        try:
            instance = self.create_server()
            self.manager.servers_client.stop(instance['id'])
            waiters.wait_for_server_status(self.manager.servers_client,
                                           instance['id'], 'SHUTOFF')
            cap_image_name = data_utils.rand_name('test-cap-img')
            captured_image = self.\
                _capture_image_from_server(instance['id'],
                                           name=cap_image_name,
                                           wait_until='ACTIVE')
            if cap_image_name != captured_image['name'] or \
                    captured_image['status'] != 'ACTIVE':
                raise exceptions.ImageNotCorrectlyCaptured
        except Exception as e:
            LOG.warning(_LW('Test capture image failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise e
        finally:
            self.delete_all()


if __name__ == '__main__':
    server = CaptureImageTest()
    server.test_capture_image()
