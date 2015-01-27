import os, sys
import math

ssd_phy_blocks_per_channel = 8192
ssd_channel = 44

def ssd_free_blocks():
    channel = 0
    free_blocks = [None] * ssd_channel
    while channel < ssd_channel:
        res = []
        command = 'ssd_read_table -c ' + str(channel) + ' -t 1 | grep \'invalid physical block\''
        result = os.popen(command)
        res = result.read().split('\n')
        del res[-1]

        bad_blocks = len(res)
        free_blocks[channel] = ssd_phy_blocks_per_channel - bad_blocks

        channel += 1

    return free_blocks
            
if '__main__' == __name__:
    free_blocks = ssd_free_blocks()
#    print free_blocks
    for channel in range(0, 44):
        if free_blocks[channel] > 8000:
            free_blocks[channel] = 8000

        print 'channel: %d free blocks: %d' % (channel, free_blocks[channel])
