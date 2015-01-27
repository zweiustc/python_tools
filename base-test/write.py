import os
list = ['fds' '897e3']
file = open('./cloud-init.log', 'a')
file.write("mount_callback_umount\n")
file.write(str(list))
file.write('\n')
sa ='sda'
file.write("mount : %s" % sa)
file.close()
