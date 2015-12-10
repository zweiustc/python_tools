#!/usr/bin/env python
from pysphere import VIServer

server = VIServer()
    server.connect("192.168.1.1", 'admin@vsphere.local', 'Password')
    vmpathlist = server.get_registered_vms()
    print vmpathlist
    for path in vmpathlist:
        vm = server.get_vm_by_path(path)
        print vm.get_properties(False)
