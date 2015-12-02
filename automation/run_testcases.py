import getopt
import sys

from test_discover import test_discover


def Usage():
    print 'run_testcases.py usage:'
    print '-h,--help: print help message.'
    print '-v, --version: print script version'
    print '-l, --list: list all testcases'
    print '-r, --runcase: run single case'
    print '-a, --runallcases: run all cases'
    print '-c, --cycle: cycle count when run cases'


def Version():
    print('run testcases 1.0.0')


def List():
    case_handler = test_discover.TestDiscover()
    cases = case_handler.load_tests()
    for case in cases:
        msg = 'TESTCASE: %s' % case
        print msg


def run_case(testcase, cycle=1):
    case_handler = test_discover.TestDiscover()
    case_handler.run_test(testcase, cycle=cycle)


def run_all_cases(cycle=1):
    case_handler = test_discover.TestDiscover()
    case_handler.run_all_tests(cycle=cycle)


def main(argv):
    try:
        opts, args = getopt.getopt(argv[1:], 'hvlar:c:',
                                   ['help', 'version', 'list',
                                    'runcase=', 'runallcases', 'cycle='])
    except getopt.GetoptError as err:
        print str(err)
        Usage()
        sys.exit(2)

    cycle = 1
    action = None
    parameter = None
    for flag, para in opts:
        if flag in ('-h', '--help'):
            Usage()
            sys.exit(1)
        elif flag in ('-v', '--version'):
            Version()
            sys.exit(0)
        elif flag in ('-l', '--list'):
            List()
            sys.exit(0)
        elif flag in ('-r', '--runcase'):
            action = 'runcase'
            parameter = para
        elif flag in ('-a', '--runallcases'):
            action = 'runallcases'
        elif flag in ('-c', '--cycle'):
            cycle = para
        else:
            print 'unhandled option'
            sys.exit(3)

    if action is None:
        sys.exit(0)

    if parameter is not None and action == 'runcase':
        run_case(parameter, cycle=cycle)
    elif parameter is None and action == 'runallcases':
        run_all_cases(cycle=cycle)
    else:
        print('unhandled option')
        sys.exit(3)


if __name__ == '__main__':
    main(sys.argv)
