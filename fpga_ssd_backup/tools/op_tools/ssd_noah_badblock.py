import os, sys
import math

def max_bad_blocks():
    max = 0
    channel = 0
    while channel < 44:
        plane = [0] * 4
        res = []
        command = 'ssd_read_table -c ' + str(channel) + ' -t 1 | grep \'invalid physical block\''
        result = os.popen(command)
        res = result.read().split('\n')
        del res[-1]

        for each in res:
            block = int(each.split(' ')[-1] )
            if block < 4096:
                if block % 2 == 0:
                    plane[0] += 1
                else:
                    plane[1] += 1
            else:
                if block % 2 == 0:
                    plane[2] += 1
                else:
                    plane[3] += 1

        i = 0
        while i < 4:
            if max < plane[i]:
                max = plane[i]
            i += 1

        channel += 1

    return max
            
if '__main__' == __name__:
    blocks = max_bad_blocks()
    print int(math.ceil(blocks * 100.0 / 2048))
