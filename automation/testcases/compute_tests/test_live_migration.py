# !/usr/bin/python
from automation.common import exceptions
from automation import config
from automation.i18n import _LE
from automation.i18n import _LI
from oslo_log import log as logging
from testcases.compute_tests.base_compute import Base

CONF = config.CONF
LOG = logging.getLogger(__name__)


class LiveMigrationTest(Base):

    def live_migration_test(self):
        try:
            compute = Base()
        except Exception as err:
            LOG.error(_LE('init env failed for %s'), str(err))
            raise exceptions.ServerFault

        try:
            instance = compute.create_server()
            compute.show_server(instance['id'])
            orig_host = compute.get_server_host(instance['id'])

            # when migrate, the status transfer from MIGRATING
            # TO ACTIVE
            # do not use block migration and no host is assigned
            self.manager.servers_client.live_migrate_server(
                instance['id'], None, False)

            compute._check_server_status_interval(instance['id'],
                                                  'ACTIVE', interval=10,
                                                  timeout=60)

            # check the host attributes
            curr_host = compute.get_server_host(instance['id'])
            LOG.debug(('the origin host %(orig)s the current host'
                       ' %(curr)s'),
                      {'orig': orig_host, 'curr': curr_host})
            if curr_host is not None and orig_host == curr_host:
                raise exceptions.ServerMigrateFailed()
            LOG.info(_LI('live migrate of %s successful'), instance['id'])
        except Exception as err:
            LOG.error(_LE('migrate server failed for %s'), str(err))
            compute.delete_all()
            raise exceptions.ServerMigrateFailed()
        finally:
            compute.delete_all()


if __name__ == '__main__':
    migration_test = LiveMigrationTest()
    migration_test.live_migration_test()
