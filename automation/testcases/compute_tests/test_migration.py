# !/usr/bin/python
from automation.common import exceptions
from automation import config
from automation.i18n import _LE
from automation.i18n import _LI
from oslo_log import log as logging
from testcases.compute_tests.base_compute import Base

CONF = config.CONF
LOG = logging.getLogger(__name__)


class MigrationTest(Base):

    def migration_test(self):
        try:
            compute = Base()
        except Exception as err:
            LOG.error(_LE('init env failed for %s'), str(err))
            raise exceptions.ServerFault

        try:
            instance = compute.create_server()
            compute.show_server(instance['id'])
            orig_host = compute.get_server_host(instance['id'])

            # power off the instance, prepare for migration
            compute.power_off(instance['id'])
            self.manager.servers_client.migrate_server(instance['id'])
            migrations = self.manager.migrations_client.list_migrations()
            LOG.info(_LI('the current migration list : %s'), migrations)

            # when migrate, the status transfer from RESIZE
            # VERIFY_RESIZE to SHUTOFF or ACTIVE
            compute._check_server_status_interval(
                instance['id'], 'VERIFY_RESIZE', interval=10, timeout=200)
            self.manager.servers_client.confirm_resize(instance['id'])
            compute._check_server_status_interval(
                instance['id'], 'SHUTOFF', interval=10, timeout=60)

            # check the host attributes
            curr_host = compute.get_server_host(instance['id'])
            if curr_host is not None and orig_host == curr_host:
                raise exceptions.ServerMigrateFailed()
            LOG.info(('cold migrate of %s successful'), instance['id'])
        except Exception as err:
            LOG.error(_LE('migrate server failed for %s'), str(err))
            compute.delete_all()
            raise exceptions.ServerMigrateFailed()
        finally:
            compute.delete_all()


if __name__ == '__main__':
    migration_test = MigrationTest()
    migration_test.migration_test()
