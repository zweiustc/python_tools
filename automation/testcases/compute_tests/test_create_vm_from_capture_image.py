# !/usr/bin/python
import base_compute as base
import uuid

from automation.common import exceptions
from automation.common.utils import data_utils
from automation.common import waiters
from automation import config
from automation.i18n import _LW

from oslo_log import log as logging
import traceback

CONF = config.CONF
LOG = logging.getLogger(__name__)


class CaptureImageTest(base.Base):

    def _create_server_from_image(self, image_id):
        server_name = str(uuid.uuid4())
        image_ref = image_id
        flavor_ref = self._get_flavor_id()
        network_id = self._get_network_id()
        network_ref = [{"uuid": network_id}]
        try:
            vm = self.manager.servers_client.\
                create_server(server_name,
                              image_ref,
                              flavor_ref,
                              networks=network_ref)
            self.servers.append(vm['id'])
            self._check_server_status_interval(vm['id'], 'ACTIVE')
            return self.manager.servers_client.show_server(vm['id'])
        except exceptions.ServerActionSucceed:
            raise

    def test_create_instance_from_captured_image(self):
        try:
            instance = self.create_server()
            self.manager.servers_client.stop(instance['id'])
            waiters.wait_for_server_status(self.manager.servers_client,
                                           instance['id'], 'SHUTOFF')
            captured_image_name = data_utils.rand_name('test-cap-img')
            captured_image = self.\
                _capture_image_from_server(instance['id'],
                                           name=captured_image_name,
                                           wait_until='ACTIVE')
            vm = self._create_server_from_image(captured_image['id'])
            if vm['status'] != 'ACTIVE':
                raise exceptions.ServerNotCorrectlyCreatedFromCapturedImage
        except Exception as e:
            LOG.warning(_LW('Test create vm from captured image failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise e
        finally:
            self.delete_all()


if __name__ == '__main__':
    server = CaptureImageTest()
    server.test_create_instance_from_captured_image()
