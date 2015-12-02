from automation import config
from automation.i18n import _LW
from oslo_log import log as logging

_CONF = config.CONF
LOG = logging.getLogger(__name__)


class NovaTest(object):

    pp = "debug detail"

    def test_server(self):
        LOG.info('GKL-write some log')
        LOG.debug('debug information %s' % self.pp)
        LOG.exception('some exception')
        LOG.warn("Unable to delete volume")
        LOG.warn(_LW("Unable to delete volume"))

NovaTest().test_server()
