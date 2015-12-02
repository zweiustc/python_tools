# !/usr/bin/python
import traceback

from automation.common import exceptions
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.volume_tests.base import Base

CONF = config.CONF
LOG = logging.getLogger(__name__)


if __name__ == '__main__':
    try:
        volume = Base()
    except Exception as e:
        LOG.warning(_LW('Init test env failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
        raise exceptions.InitFailed
    try:
        volume_created = volume.create_volume()
        volume.extend_volume(volume_created['id'])
    except Exception as e:
        LOG.warning(_LW('Test create volume failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
        raise e
    finally:
        volume.delete_all()
