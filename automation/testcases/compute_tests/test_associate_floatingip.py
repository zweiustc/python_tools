# !/usr/bin/python
import base_compute as base
import traceback

from automation.common import exceptions
from automation import config
from automation.i18n import _LW
from oslo_log import log as logging

CONF = config.CONF
LOG = logging.getLogger(__name__)


class FloatingIpTest(base.Base):

    def test_associate_floatingip(self):
        try:
            instance = self.create_server()
            fp_body = self.manager.floating_ips_client.\
                create_floating_ip('external')
            floating_ip_id_allocated = fp_body['id']
            floating_ip_ip_allocated = fp_body['ip']
            self.manager.floating_ips_client.\
                associate_floating_ip_to_server(floating_ip_ip_allocated,
                                                instance['id'])
            # Check instance_id in the floating_ip body
            body = self.manager.floating_ips_client.\
                show_floating_ip(floating_ip_id_allocated)
            if instance['id'] != body['instance_id']:
                raise exceptions.FloatingIpNotCorrectlyAssociated()
        except Exception as e:
            LOG.warning(_LW('Test associate floating ip failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise e
        finally:
            # Disassociation of floating IP that was associated in this method
            try:
                self.manager.\
                    floating_ips_client.\
                    disassociate_floating_ip_from_server(
                        floating_ip_ip_allocated,
                        instance['id'])
            except Exception as e:
                LOG.warning(_LW('Disassociate floating ip failed, '
                                'msg: %(msg)s, traceback: %(tb)s.'),
                            {'msg': e, 'tb': traceback.format_exc()})

            try:
                self.manager.floating_ips_client.delete_floating_ip(
                    floating_ip_id_allocated)
            except Exception as e:
                LOG.warning(_LW('Delete floating ip failed, '
                                'msg: %(msg)s, traceback: %(tb)s.'),
                            {'msg': e, 'tb': traceback.format_exc()})
            self.delete_all()


if __name__ == '__main__':
    server = FloatingIpTest()
    server.test_associate_floatingip()
