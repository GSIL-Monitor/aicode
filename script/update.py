#!/usr/local/bin/python
#coding:utf-8

import datetime
import time
from fabric.context_managers import *
from fabric.contrib.console import confirm
from fabric.colors import *
from fabric.api  import *

env.warn_only = True

#local info
env.local_package_dir='/usr/local/src/modules/module/http_server/bin'
env.time=time.strftime("%Y%m%d_%H_%M_%S")
env.local_bakcup_dir='/tmp'
env.local_package_name='access.cgi'
#remote info
env.hosts=['47.91.151.24', '47.88.33.242']
env.port='22'
env.user='root'
env.password="xxxxx"
env.remote_package_dir='/data/platform/http/bin'  

@task
@runs_once
def backup_task():
   print red("begin tar file...")
   with lcd ("%s" %env.local_package_dir):
          local("tar zcvf %s-%s.tar.gz %s" %(env.local_package_name,env.time,env.local_package_name))
   print blue("end tar.")
   
   print yellow('===========================')
   

@task
def put_task():
   print red("begin upload file...")
   with lcd ("%s" %env.local_package_dir):
     put('%s-%s.tar.gz' %(env.local_package_name,env.time),'%s/%s-%s.tar.gz' %(env.remote_package_dir,env.local_package_name,env.time))
   print blue('end upload')
   
   print yellow('===========================')
   
   print red("begin stop remote service...")
   with cd("%s" %(env.remote_package_dir)):
     run ('./stop.sh')
   print blue('end stop remote service')
   
   print yellow('===========================')
   
   print red("begin deploy file...")
   run ('tar zxf %s/%s-%s.tar.gz -C %s' %(env.remote_package_dir,env.local_package_name,env.time,env.remote_package_dir))
   print blue('end deploy file')
   
   print yellow('===========================')
   
   print red("begin run remote service...")
   with cd("%s" %(env.remote_package_dir)):
     run ('./runhttp.sh')
   print blue('end run remote service')
   
@task
def start():
   backup_task()
   put_task()
   