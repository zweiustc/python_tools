# !/usr/bin/python
import traceback

from automation.common import exceptions
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.compute_tests.base_compute import Base

CONF = config.CONF
LOG = logging.getLogger(__name__)

instance_types = [str(i) for i in range(1, 6)]


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
        original_host = server_details.get("OS-EXT-SRV-ATTR:host")
        original_flavor = server_details.get("flavor").get("id")
        server.resize(server_created['id'], CONF.compute.flavor_name_alt)
        server_details = server.show_server(server_created['id'])
        new_host = server_details.get("OS-EXT-SRV-ATTR:host")
        if new_host == original_host:
            raise exceptions.ServerResizeFailed
    except Exception as e:
        LOG.warning(_LW('Test resize server failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
        raise e
    finally:
        server.delete_all()
