# Copyright 2013 IBM Corp.
#
#    Licensed under the Apache License, Version 2.0 (the "License"); you may
#    not use this file except in compliance with the License. You may obtain
#    a copy of the License at
#
#         http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
#    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
#    License for the specific language governing permissions and limitations
#    under the License.

import os

from automation import config
from automation.i18n import _LE
from automation.i18n import _LI
from oslo_log import log as logging

_CONF = config.CONF
LOG = logging.getLogger(__name__)


class TestDiscover(object):

    _LONG_RUN_TIMES = 1000000

    def __init__(self, path="./testcases/"):
        self.test_path = path

    def load_tests(self):
        testcases = []
        for parent, dirnames, filenames in os.walk(self.test_path):
            for filename in filenames:
                if filename.startswith('test_'):
                    case = os.path.join(parent, filename)
                    case = os.path.realpath(case)
                    testcases.append(case)
        return testcases

    def list_tests(self):
        testcases_path = self.load_tests()
        testcases = []
        for case in testcases_path:
            testcases.append(os.path.basename(case))
        return testcases

    def run_all_tests(self, cycle=1):
        testcases = self.load_tests()
        run_times = cycle if cycle > 0 else self._LONG_RUN_TIMES
        if isinstance(run_times, basestring):
            run_times = int(run_times)

        path = os.path.abspath(
            os.path.dirname(os.path.dirname(__file__)))
        python_path = os.environ.get("PYTHONPATH")
        if not python_path:
            python_path = path
        else:
            python_path += ';' + path
        os.environ['PYTHONPATH'] = python_path

        while run_times > 0:
            total_cases_num = len(testcases)
            failed_cases_num = 0
            failed_cases = []
            for case in testcases:
                try:
                    cmd = "python " + case
                    LOG.info(_LI('TESTCASE begin to run: %s') % case)
                    ret = os.system(cmd)
                    if ret == 0:
                        LOG.info(('TESTCASE run successfully: %s'), case)
                    else:
                        raise
                except Exception:
                    LOG.error(_LE('TESTCASE failed: %s'), case)
                    failed_cases.append(case)
                    failed_cases_num += 1
            LOG.info(_LI('RUN ALL CASES Total: %(total)s Fail: %(fail)s'),
                     {'total': total_cases_num, 'fail': failed_cases_num})
            for case in failed_cases:
                LOG.info(_LI('RUN ALL CASES FAILED: %s'), case)
            run_times -= 1

    def run_test(self, testcase=None, cycle=1):
        if testcase is None:
            return
        run_times = cycle if cycle > 0 else self._LONG_RUN_TIMES
        if isinstance(run_times, basestring):
            run_times = int(run_times)

        path = os.path.abspath(
            os.path.dirname(os.path.dirname(__file__)))
        python_path = os.environ.get("PYTHONPATH")
        if not python_path:
            python_path = path
        else:
            python_path += ';' + path
        os.environ['PYTHONPATH'] = python_path

        while run_times > 0:
            try:
                cmd = "python " + testcase

                LOG.info(('TESTCASE begin to run: %s'), testcase)
                ret = os.system(cmd)
                if ret == 0:
                    LOG.info(_LI('TESTCASE run successfully: %s'),
                             testcase)
                else:
                    raise
            except Exception:
                LOG.error(('TESTCASE failed: %s'), testcase)
            run_times = run_times - 1


if __name__ == "__main__":
    test = TestDiscover('./../testcases')
    cases = test.load_tests()
    print(cases)
    cases = test.list_tests()
    print(cases)
