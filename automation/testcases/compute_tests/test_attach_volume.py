import base_compute as base
import traceback
import uuid

from automation.common import exceptions
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging

CONF = config.CONF
LOG = logging.getLogger(__name__)


class AttachVolumeTest(base.Base):

    def __init__(self):
        super(AttachVolumeTest, self).__init__()
        self.volume_client = self.manager.volumes_v2_client

    def test_attach_volume(self):
        volume = None
        try:
            instance = self.create_server()
            name = "vol-" + self.__class__.__name__ + "-" + \
                   str(uuid.uuid4())[:5]
            kwargs = dict()
            kwargs['name'] = name
            volume = self.volume_client.create_volume(1, **kwargs)
            self.volume_client.wait_for_volume_status(volume['id'],
                                                      'available')

            # Volume is attached and detached successfully from an instance
            mount_point = '/dev/vdc'
            self.volume_client.attach_volume(volume['id'],
                                             instance['id'],
                                             mount_point)
            self.volume_client.wait_for_volume_status(volume['id'], 'in-use')
            self.volume_client.detach_volume(volume['id'])
            self.volume_client.wait_for_volume_status(volume['id'],
                                                      'available')
        except Exception as e:
            LOG.warning(_LW('Test volume attach failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise e
        finally:
            is_successfully_deleted = True
            try:
                self.delete_all()
            except Exception as e:
                LOG.warning(_LW('Server delete failed: %s.'), e)
                is_successfully_deleted = False

            try:
                if volume is not None:
                    self.volume_client.delete_volume(volume['id'])
            except exceptions.NotFound:
                pass
            except Exception as e:
                LOG.warning(_LW('Delete volume %(volume)s failed, '
                                'msg: %(msg)s, traceback: %(tb)s.'),
                            {'volume': volume['id'],
                             'msg': e, 'tb': traceback.format_exc()})
                is_successfully_deleted = False

            if not is_successfully_deleted:
                raise exceptions.DeleteException


if __name__ == '__main__':
    server = AttachVolumeTest()
    server.test_attach_volume()
