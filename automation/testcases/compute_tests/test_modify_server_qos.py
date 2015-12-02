# !/usr/bin/python
import sys
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
        server.power_off(server_created['id'])
        server_current_qos = str(server.get_server_current_qos(
            server_created['id']))
        qos_list = server.get_qos_types()
        qos_ids = []
        for qos in qos_list:
            qos_ids.append(str(qos['id']))
        if server_current_qos in qos_ids:
            qos_ids.remove(server_current_qos)
        if not qos_ids:
            LOG.warning(_LW('Qos not configured in this env, skip.'))
            sys.exit(0)
        new_qos_id = qos_ids[0]
        server.set_server_network_qos(server_created['id'], server_current_qos,
                                      new_qos_id)
        new_qos = str(server.get_server_current_qos(
            server_created['id']))
        if new_qos != new_qos_id:
            raise exceptions.ServerNetworkQosChangeFailed
    except Exception as e:
        LOG.warning(_LW('Test modify qos failed, msg: %(msg)s, traceback: '
                        '%(tb)s.'), {'msg': e, 'tb': traceback.format_exc()})
        raise e
    finally:
        server.delete_all()
