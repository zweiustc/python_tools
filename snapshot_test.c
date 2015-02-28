#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>

int main(int argc, char ** argv) {
    virConnectPtr conn = virConnectOpen(NULL);
    if(conn == NULL) {
        printf("error connecting qemu driver\n");
        exit(1);
    }   
    virDomainPtr vm_ptr = virDomainLookupByName(conn, "baidu-6.5");
    if(vm_ptr == NULL) {
        printf("error finding domain\n");
        virConnectClose(conn);
        exit(1);
    }   

    char buf[1024] = "<domainsnapshot>\n  <name>00a</name>\n  <disks>\n    <disk name='hda' snapshot='external'>\n      <source file='/home/li/work/img-test/tmp/006'/>\n    </disk>\n  </disks>\n</domainsnapshot>\n";
    if (NULL == virDomainSnapshotCreateXML(vm_ptr, buf, VIR_DOMAIN_SNAPSHOT_CREATE_NO_METADATA | VIR_DOMAIN_SNAPSHOT_CREATE_DISK_ONLY )){ 
                    printf("the error\n");
    }   
    //virTestCommand();

    printf("=================\n");
    virDomainFree(vm_ptr);
    virConnectClose(conn);
    return 0;
}
