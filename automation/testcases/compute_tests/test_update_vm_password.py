# !/usr/bin/python
import base_compute as base
import traceback

from automation.common import exceptions
from automation.common import waiters
from automation import config
from automation.i18n import _LI
from automation.i18n import _LW
from oslo_log import log as logging

CONF = config.CONF
LOG = logging.getLogger(__name__)


class UpdateVMPassword(base.Base):

    def test_update_vm_password(self):
        try:
            instance = self.create_server()
            admin_pass = "adminPass"
            self.manager.servers_client.change_password(instance['id'],
                                                        admin_pass)
            waiters.wait_for_server_status(self.manager.servers_client,
                                           instance['id'], 'ACTIVE')
            server_detail = self.manager.servers_client.\
                show_server(instance['id'])
            vm_name = server_detail.get("name")
            fp_body = self.manager.floating_ips_client.\
                create_floating_ip('external')
            floating_ip_id_allocated = fp_body['id']
            floating_ip_ip_allocated = fp_body['ip']
            self.manager.floating_ips_client.\
                associate_floating_ip_to_server(floating_ip_ip_allocated,
                                                instance['id'])

            self.manager.servers_client.\
                add_security_group(instance['id'],
                                   CONF.compute.security_group)
            cmd = "hostname"
            f = open('/root/.ssh/known_hosts', 'w')
            f.truncate()
            ret = self._check_server_start_status_interval(
                floating_ip_ip_allocated,
                admin_pass,
                cmd)
            if vm_name not in str(ret):
                raise exceptions.PasswordNotCorrectlyUpdated
            else:
                LOG.info(_LI("Congratulations! VM password was successfully "
                             "updated!"))
        except Exception as e:
            LOG.warning(_LW('Update vm password failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise e
        finally:
            try:
                self.manager.floating_ips_client.\
                    disassociate_floating_ip_from_server(
                        floating_ip_ip_allocated,
                        instance['id'])
            except Exception as e:
                LOG.warning(_LW('Disassociate floating ip failed, '
                                'msg: %(msg)s, traceback: %(tb)s.'),
                            {'msg': e, 'tb': traceback.format_exc()})
            try:
                self.manager.floating_ips_client.\
                    delete_floating_ip(floating_ip_id_allocated)
            except Exception as e:
                LOG.warning(_LW('Delete floating ip failed, '
                                'msg: %(msg)s, traceback: %(tb)s.'),
                            {'msg': e, 'tb': traceback.format_exc()})
            self.delete_all()

if __name__ == '__main__':
    server = UpdateVMPassword()
    server.test_update_vm_password()
