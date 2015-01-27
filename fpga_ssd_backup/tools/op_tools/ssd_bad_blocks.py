import os, sys
import math

def max_bad_blocks():
    channel = 0
    bad_blocks = [None] * 44
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
            bad_blocks[channel] = plane

        channel += 1

    return bad_blocks
            
if '__main__' == __name__:
    blocks = max_bad_blocks()
#    print blocks
    max = 0
    max_channel = 0
    max_plane = 0
    plane = []
    for ch in range(0, 44):
        plane = blocks[ch]
        for pl in range(0, 4):
            if max < plane[pl]:
                max = plane[pl]
                max_channel = ch
                max_plane = pl

#    print int(math.ceil(max * 100.0 / 2048))
    print "channel: " + str(max_channel) + " plane: " + str(max_plane) + " max bad blocks: " + str(max)
