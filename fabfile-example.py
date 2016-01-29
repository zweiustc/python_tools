#/usr/bin/python
# Use fabric we should install first.
# In the example I install fabric with source code.

import os
from fabric.api import env,local,cd,lcd,put,run
from fabric.api import roles
from fabric.api import parallel, serial

env.roledefs = {
    'controller': ['root@10.160.57.66:22', 
                   'root@10.160.57.67:22', 
                   'root@10.160.57.68:22']
}
env.passwords = {
    'root@10.160.57.66:22': 'password',
    'root@10.160.57.67:22': 'password',
    'root@10.160.57.68:22': 'password',
}

# parallel decorator make the task run parallelly
# serial decorator make the task run serially
# the default mode is serial

@parallel
@roles('controller')
def sync_update():
    abs_dir = '/root/git/master/nova'

    local_file = 'nova/exception.py'
    local_file = os.path.join(abs_dir, local_file) 
    remote_dir = '/usr/lib/python2.7/site-packages/nova/exception.py'
    put(local_file, remote_dir)

def put_sshkey():
    with cd('/tmp'):
        put('id_rsa.pub.master', 'id_rsa.pub.master')
        run('cat id_rsa.pub.master >> /root/.ssh/authorized_keys')

def pyclean():
    local("pyclean .")      # local 函数执行本地命令


def deploy():
    pyclean()

    local_app_dir = "~/workspace/projects/"
    remote_app_dir = "~/projects"

    # lcd 是 「local cd」，cd 是在远程服务器执行 cd
    with lcd(local_app_dir), cd(remote_app_dir):
        # 1. backup
        run("rm -rf SomeProj.bak")          # run 是在远程服务器上执行命令
        run("mv SomeProj SomeProj.bak")

        # 2. transfer
        d = os.path.join(remote_app_dir, "SomeProj")
        run("mkdir -p %s" % d)
        # put 把本地文件传输到远程（ SFTP (SSH File Transfer Protocol) 协议）
        put("SomeProj/*", "SomeProj")

        # 3. replace site_settings.py
        for subdir in ["apps", "admin", "core"]:
            src = "SomeProj.bak/%s/configs/site_settings.py" % subdir
            dest = "SomeProj/%s/configs/site_settings.py" % subdir
            cmd = "cp %s %s" % (src, dest)
            run(cmd)
