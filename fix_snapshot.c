#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <libvirt/libvirt.h>
#include <libvirt/virterror.h>

#define instance_name_max_len 100
#define xml_info_len 4096

int main(int argc, char ** argv) {
    char * instance;
    virDomainSnapshotPtr snapshot;
    instance = (char *)malloc(instance_name_max_len);

    //get the instance name
    printf("please input the instance uuid:");
    scanf("%s", instance);
    printf("deal with instance: %s\n", instance);
    virConnectPtr conn;
    conn = virConnectOpen("qemu:///system");
    if(conn == NULL) {
        printf("failed to connect qemu driver\n");
        exit(1);
    }

    //find the instance object
    virDomainPtr instance_ptr = virDomainLookupByUUID(conn, instance);
    if(instance_ptr ==  NULL) {
        printf("could not found the instance %s\n", instance);
        virConnectClose(conn);
        exit(1);
    }

    char * xml_info;
    xml_info = (char *)malloc(xml_info_len);
    char disk_name[100] = "<domainsnapshot>\n  <disks>\n    <disk name=\"vda\" snapshot=\"external\" type=\"file\">\n      ";
    char source_file[300];
    sprintf(source_file, "<source file=\"/home/bcc/instances/%s/disk\"/>\n    </disk>\n    ", instance);
    char extra_info[300] = "<disk name=\"vdb\" snapshot=\"no\"/    >\n  </disks>\n</domainsnapshot>\n";
    sprintf(xml_info, "%s%s%s", disk_name, source_file, extra_info);
    printf("xml_info: %s", xml_info);

    //close the connection
    printf("begin to create snapshot with xml info: %s\n", xml_info);
    snapshot = virDomainSnapshotCreateXML(instance_ptr, xml_info,
            VIR_DOMAIN_SNAPSHOT_CREATE_NO_METADATA |
            VIR_DOMAIN_SNAPSHOT_CREATE_DISK_ONLY |
            VIR_DOMAIN_SNAPSHOT_CREATE_REUSE_EXT);
    if (snapshot != NULL)
        printf("create successfully!");
    virConnectClose(conn);
    return 0;
}
