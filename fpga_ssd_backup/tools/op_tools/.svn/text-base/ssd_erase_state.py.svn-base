import os, sys
import math

def erase_info_blocks():
    channel = 0
    erase_info_ssd = [None] * 44
   
    while channel < 44:
        res = []
        bad_erase = 0
        max_erase = 0
        min_erase = 0
        avg_erase = 0
        total_erase = 0
        erase_info = [0] * 4 

        #grep the erase times per block
        command = './ssd_read_table -c ' + str(channel) + ' -t 1 -d | grep \'erase entry info: \' | awk -F \':\' \'{print $2}\''
        result = os.popen(command)
        res = result.read().split('\n')
        del res[-1]   # del \n
        
        #calculate the max bad min and average erase_times
        for each in res:
            erase_times = int(each.split('\t')[-1] ) #get erase times
            total_erase += erase_times
            if erase_times > 3000:
                bad_erase += 1
            if max_erase < erase_times:
                max_erase = erase_times
            if min_erase > erase_times:
                min_erase = erase_times      
        block_num = len(res)    
        if block_num > 0:
            avg_erase = total_erase/block_num
      
            
        #packet the results
        erase_info[0] = bad_erase
        erase_info[1] = max_erase
        erase_info[2] = min_erase
        erase_info[3] = avg_erase
        erase_info_ssd[channel] = erase_info    

        channel += 1

    return erase_info_ssd
            
if '__main__' == __name__:
    erase_info_ssd = erase_info_blocks()

#    print blocks
    erase_info = []
    for ch in range(0, 44):
        erase_info = erase_info_ssd[ch]
        print "channel = " + str(ch) + "  bad erase blocks(> 3000) = " + str(erase_info[0]) + "  max_erase = " + \
        str(erase_info[1]) + "  min_erase =  " + str(erase_info[2]) + "  average_erase = " + str(erase_info[3]) 
       

