# !/usr/bin/python
import traceback

from automation.common import exceptions
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.compute_tests.base_compute import Base

CONF = config.CONF
LOG = logging.getLogger(__name__)


if __name__ == '__main__':
    try:
        server = Base()
    except Exception as e:
        LOG.warning(_LW('Init test env failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
        raise exceptions.InitFailed
    try:
        server_created = server.create_server()
        server_details = server.show_server(server_created['id'])
    except Exception as e:
        LOG.warning(_LW('Test create server failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
        raise e
    finally:
        server.delete_all()
