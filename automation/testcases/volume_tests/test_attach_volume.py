# !/usr/bin/python
import traceback

from automation.common import exceptions
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.compute_tests.base_compute import Base as BaseCompute
from testcases.volume_tests.base import Base as BaseVolume

CONF = config.CONF
LOG = logging.getLogger(__name__)


if __name__ == '__main__':
    try:
        volume = BaseVolume()
        server = BaseCompute()
    except Exception as e:
        LOG.warning(_LW('Init test env failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
        raise exceptions.InitFailed
    try:
        server_created = server.create_server()
        volume_created = volume.create_volume()
        server.attach_volume(server_created['id'], volume_created['id'])
        volume.check_volume_status_interval(volume_created['id'], 'in-use')
        server.detach_volume(server_created['id'], volume_created['id'])
        volume.check_volume_status_interval(volume_created['id'], 'available')
    except Exception as e:
        LOG.warning(_LW('Test create volume failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
        raise e
    finally:
        is_successfully_deleted = True
        try:
            server.delete_all()
        except Exception as e:
            LOG.warning(_LW('During test attach volume server delete failed.'))
            is_successfully_deleted = False
        try:
            volume.delete_all()
        except Exception as e:
            LOG.warning(_LW('During test attach volume volume delete failed.'))
            is_successfully_deleted = False
        if not is_successfully_deleted:
            raise exceptions.DeleteException
