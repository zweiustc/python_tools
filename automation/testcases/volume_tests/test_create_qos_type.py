# !/usr/bin/python
import traceback

from automation.common import exceptions
from automation import config
from automation.i18n import _LE
from automation.i18n import _LW
from oslo_log import log as logging
from testcases.volume_tests.base import Base


CONF = config.CONF
LOG = logging.getLogger(__name__)


class TestQosType(Base):

    __disk_qos_low = {"read_iops_sec": "100",
                      "write_iops_sec": "50",
                      "read_bytes_sec": "409600",
                      "write_bytes_sec": "204800"}

    __disk_qos_medium = {"read_iops_sec": "200",
                         "write_iops_sec": "100",
                         "read_bytes_sec": "819200",
                         "write_bytes_sec": "409600"}

    def test_create_qos_type(self):
        try:
            volume = Base()
        except Exception as e:
            LOG.warning(_LW('Init test env failed, msg: %(msg)s, '
                            'traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise exceptions.InitFailed

        try:
            qos_type_created = \
                volume.create_test_qos_specs(**self.__disk_qos_low)
        except Exception as e:
            LOG.warning(_LW('Create qos type for volume tests failed, '
                            'msg: %(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            volume.delete_all()
            raise

        try:
            volume.show_qos_type(qos_type_created['id'])
        except Exception as e:
            LOG.warning(_LE('show volume type failed, msg: '
                            '%(msg)s, traceback: %(tb)s.'),
                        {'msg': e, 'tb': traceback.format_exc()})
            raise
        finally:
            volume.delete_all()


if __name__ == "__main__":
    volume_type_test = TestQosType()
    volume_type_test.test_create_qos_type()
