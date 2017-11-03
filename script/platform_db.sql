CREATE DATABASE IF NOT EXISTS PlatformDB DEFAULT CHARSET utf8 COLLATE utf8_general_ci;

use PlatformDB;

DROP TABLE IF EXISTS `t_user_info`;
CREATE TABLE `t_user_info` (
  `id` varchar(36) NOT NULL,
  `userid` varchar(100) NOT NULL,
  `username` varchar(100) NOT NULL,
  `userpassword` varchar(100) DEFAULT NULL,
  `typeinfo` int(11) NOT NULL DEFAULT '0',
  `aliasname` varchar(128), #用户别名
  `email` varchar(128) NOT NULL, #用户邮箱
  `apptype` int(11) NOT NULL,
  `createdate` datetime NOT NULL,
  `status` int(11) NOT NULL DEFAULT '0', #0正常，1删除
  `extend` varchar(4000) DEFAULT NULL,
  `exceptionstate` int(11) DEFAULT '0', #0-正常
  PRIMARY KEY (`id`),
  INDEX index_ref1(userid),
  INDEX index_ref2(username, createdate),
  INDEX index_ref3(status),
  INDEX index_ref4(createdate)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `t_device_info`;
CREATE TABLE `t_device_info` (
  `id` varchar(36) NOT NULL,
  `deviceid` varchar(100) NOT NULL,
  `devicename` varchar(100) NOT NULL,
  `devicepassword` varchar(100) DEFAULT NULL,
  `typeinfo` int(11) NOT NULL DEFAULT '0',
  `p2pid` varchar(50),
  `domainname` varchar(100),
  `createdate` datetime NOT NULL,
  `status` int(11) NOT NULL DEFAULT '0',
  `innerinfo` varchar(4000) DEFAULT NULL,
  `extend` varchar(4000) DEFAULT NULL,
  PRIMARY KEY (`id`),
  INDEX index_ref1(deviceid),
  INDEX index_ref2(devicename, createdate),
  INDEX index_ref3(status),
  INDEX index_ref4(createdate)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `t_user_device_relation`;
CREATE TABLE `t_user_device_relation` (
  `id` varchar(36) NOT NULL,
  `userid` varchar(100) NOT NULL,
  `deviceid` varchar(100) NOT NULL,
  `relation` int(11) NOT NULL DEFAULT '0', #关系包括，拥有0、被分享1、分享中2、转移3，目前只用0、1、2
  `devicekeyid` varchar(36) NOT NULL, #设备信息表主键-id
  `devicename` varchar(100) DEFAULT '', #共享设备的名称
  `begindate` datetime NOT NULL,
  `enddate` datetime NOT NULL,
  `createdate` datetime NOT NULL,
  `status` int(11) NOT NULL DEFAULT '0',
  `extend` varchar(4000) DEFAULT NULL,
  PRIMARY KEY (`id`),
  INDEX index_ref1(userid, deviceid),
  INDEX index_ref2(deviceid, userid),
  INDEX index_ref3(userid, createdate),
  INDEX index_ref4(status),
  INDEX index_ref5(createdate)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `t_user_relation`;
CREATE TABLE `t_user_relation` (
  `id` varchar(36) NOT NULL,
  `userid` varchar(100) NOT NULL,
  `relation_userid` varchar(100) NOT NULL,
  `relation` int(11) NOT NULL DEFAULT '0', #目前只使用好友关系，值为0
  `createdate` datetime NOT NULL,
  `status` int(11) NOT NULL DEFAULT '0',
  `extend` varchar(4000) DEFAULT NULL,
  PRIMARY KEY (`id`),
  INDEX index_ref1(userid, relation)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


DROP TABLE IF EXISTS `t_user_operation_log`;
CREATE TABLE `t_user_operation_log` (
  `id` varchar(36) NOT NULL,
  `userid` varchar(100) NOT NULL,
  `deviceid` varchar(100) NOT NULL,
  `operationtype` int(11) NOT NULL,
  `createdate` datetime NOT NULL,
  `status` int(11) NOT NULL DEFAULT '0',
  `extend` varchar(4000) DEFAULT NULL,
  PRIMARY KEY (`id`),
  INDEX index_ref1(userid, operationtype),
  INDEX index_ref2(deviceid),
  INDEX index_ref3(userid, createdate),
  INDEX index_ref4(status),
  INDEX index_ref5(createdate)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


DROP TABLE IF EXISTS `t_storage_info`;
CREATE TABLE `t_storage_info` (
  `id` varchar(36) NOT NULL,
  `domainid` int(11) NOT NULL DEFAULT '0', #存储域，默认为0
  `sizeofspace` bigint(24) NOT NULL, #存储空间大小，统一以MB为单位
  `sizeofspaceused` bigint(24) NOT NULL #存储空间已经使用大小
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


DROP TABLE IF EXISTS `t_storage_detail_info`;
CREATE TABLE `t_storage_detail_info` (
  `id` varchar(36) NOT NULL,
  `domainid` int(11) NOT NULL DEFAULT '0', #存储域，默认为0
  `objid` varchar(100) NOT NULL, #对象id，可以是用户或者是设备
  `objtype` int(11) NOT NULL, #对象类型，0：用户，1：设备
  `storagename` varchar(100) NOT NULL DEFAULT '', #存储空间命名
  `storagetype` int(11) NOT NULL DEFAULT '0', #存储类型，默认为0，目前表示文件
  `overlaptype` int(11) NOT NULL DEFAULT '0', #覆写类型，0：不覆写，1：FIFO覆写
  `storagetimeuplimit` int(11) NOT NULL DEFAULT '0', #存储时间上限
  `storagetimedownlimit` int(11) NOT NULL DEFAULT '0', #存储时间下限
  `sizeofspaceused` int(11) NOT NULL DEFAULT '0', #存储空间已经使用大小
  `storageunittype` int(11) NOT NULL DEFAULT '0', #存储空间单元类型，0表示以MB为单位
  `begindate` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP, #生效起始时间
  `enddate` datetime NOT NULL, #若是不限制结束时间的场景，则该值填写为2199-01-01
  `status` int(11) NOT NULL DEFAULT '0',
  `extend` varchar(4000) DEFAULT ''
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


DROP TABLE IF EXISTS `t_file_info`;
CREATE TABLE `t_file_info` (
  `id` varchar(36) NOT NULL,
  `fileid` varchar(100) NOT NULL,
  `userid` varchar(100),
  `deviceid` varchar(100),
  `remotefileid` varchar(100) NOT NULL, #服务器文件ID，与fileid一一对应
  `downloadurl` varchar(1024) NOT NULL, #文件URL地址
  `filename` varchar(256) NOT NULL,
  `suffixname` varchar(32), #文件后缀名称
  `filesize` bigint(24) NOT NULL, #文件大小，单位Byte
  `businesstype` int(11) NOT NULL, #文件业务类型
  `filecreatedate` datetime NOT NULL, #文件创建日期
  `createdate` datetime NOT NULL, #本条记录生成日期
  `status` int(11) NOT NULL DEFAULT '0', #0正常，1回收站，2永久删除
  `extend` varchar(4000) DEFAULT NULL,
  PRIMARY KEY (`id`),
  INDEX index_ref1(fileid),
  INDEX index_ref2(fileid, userid),
  INDEX index_ref3(fileid, deviceid),
  INDEX index_ref4(status),
  INDEX index_ref5(createdate)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `t_configuration_info`;
CREATE TABLE `t_configuration_info` (
  `id` varchar(36) NOT NULL,
  `category` varchar(50) NOT NULL, #配置类别，如用户APP、门铃、IPC
  `subcategory` varchar(100) NOT NULL, #配置类别子项目
  `latestversion` varchar(20) DEFAULT '', #最新版本
  `description` varchar(500) DEFAULT '', #版本详情描述
  `forceversion` varchar(20) DEFAULT '', #强制升级的版本
  `filename` varchar(100) DEFAULT '',
  `fileid` varchar(256) DEFAULT '',
  `filesize` int(11),
  `filepath` varchar(256) DEFAULT '',
  `leaseduration` int(11),
  `updatedate` datetime DEFAULT CURRENT_TIMESTAMP,
  `status` int(11) NOT NULL DEFAULT '0', #0正常，1删除
  `extend` varchar(4000) DEFAULT '',
  PRIMARY KEY (`id`),
  INDEX index_ref1(category, subcategory)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `t_access_domain_info`;
CREATE TABLE `t_access_domain_info` (
  `id` varchar(36) NOT NULL,
  `countryid` varchar(16) NOT NULL,
  `areaid` varchar (16),
  `domainname` varchar(256) NOT NULL,
  `leaseduration` int(11) NOT NULL,
  `createdate` datetime NOT NULL,
  `status` int(11) NOT NULL DEFAULT '0', #0正常，1删除
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `t_device_parameter_ipc`;
CREATE TABLE `t_device_parameter_ipc` (
  `id` varchar(36) NOT NULL,
  `deviceid` varchar(100) NOT NULL,
  `devicepassword` varchar(100) DEFAULT '',
  `devicedomain` varchar(50) DEFAULT '', #设备二级域名，与设备ID一一对应
  `typeinfo` int(11) NOT NULL,
  `username` varchar(100) DEFAULT '', #用户名称
  `userpassword` varchar(50) DEFAULT '', #用户密码
  `p2pid` varchar(50) DEFAULT '', #P2PID
  `p2pserver` varchar(100) DEFAULT '', #P2P服务器
  `p2psupplier` int(11) DEFAULT '0', #P2P供应商，1-浪涛，2-尚云，3-TUTK，4-全景VR
  `p2pbuildin` int(11) DEFAULT '-1', #P2P分配方式，0-动态分配，1-设备烧录
  `licensekey` varchar(50) DEFAULT '', #尚云P2P使用
  `pushid` varchar(50) DEFAULT '', #尚云P2P2使用
  `distributor` varchar(100) DEFAULT '', #经销商
  `corpid` varchar(100) DEFAULT '',
  `dvsname` varchar(100) DEFAULT '',
  `dvsip` varchar(50) DEFAULT '',
  `dvsip2` varchar(50) DEFAULT '',
  `webport` varchar(50) DEFAULT '',
  `ctrlport` varchar(50) DEFAULT '',
  `protocol` varchar(50) DEFAULT '',
  `model` varchar(50) DEFAULT '',
  `postfrequency` varchar(50) DEFAULT '',
  `version` varchar(50) DEFAULT '',
  `devicestatus` varchar(50) DEFAULT '',
  `serverip` varchar(50) DEFAULT '',
  `serverport` varchar(50) DEFAULT '',
  `transfer` varchar(100) DEFAULT '',
  `mobileport` varchar(50) DEFAULT '',
  `channelcount` varchar(50) DEFAULT '',
  `otherproperty` varchar(1000) DEFAULT '', #其他属性
  `createdate` datetime NOT NULL,
  `updatedate` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `status` int(11) NOT NULL DEFAULT '0',
  `extend` varchar(4000) DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY (deviceid)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `t_device_parameter_doorbell`;
CREATE TABLE `t_device_parameter_doorbell` (
  `id` varchar(36) NOT NULL,
  `deviceid` varchar(100) NOT NULL,
  `devicepassword` varchar(100) DEFAULT '',
  `devicedomain` varchar(50) DEFAULT '',            #设备二级域名，与设备ID一一对应
  `typeinfo` int(11) NOT NULL,
  `username` varchar(100) DEFAULT '',               #用户名称
  `userpassword` varchar(50) DEFAULT '',            #用户密码
  `p2pserver` varchar(100) DEFAULT '',              #P2P服务器
  `p2psupplier` int(11) DEFAULT '0',                #P2P供应商，1-浪涛，2-尚云，3-TUTK，4-全景VR
  `p2pbuildin` int(11) DEFAULT '-1',                #P2P分配方式，0-动态分配，1-设备烧录
  `licensekey` varchar(50) DEFAULT '',              #尚云P2P使用
  `pushid` varchar(50) DEFAULT '',                  #尚云P2P2使用
  `distributor` varchar(100) DEFAULT '',            #经销商
  `doorbell_name` varchar(50) DEFAULT '',           #设备名称
  `serial_number` varchar(50) DEFAULT '',           #序列号
  `doorbell_p2pid` varchar(50) DEFAULT '',          #P2P ID
  `battery_capacity` varchar(50) DEFAULT '',        #电池容量
  `charging_state` varchar(50) DEFAULT '',          #充电状态
  `wifi_signal` varchar(50) DEFAULT '',             #WIFI信号
  `volume_level` varchar(50) DEFAULT '',            #音量大小
  `version_number` varchar(50) DEFAULT '',          #版本号信息
  `channel_number` varchar(50) DEFAULT '',          #通道号
  `coding_type` varchar(50) DEFAULT '',             #编码类型
  `pir_alarm_swtich` varchar(50) DEFAULT '',        #PIR报警开关
  `doorbell_switch` varchar(50) DEFAULT '',         #门铃开关
  `pir_alarm_level` varchar(50) DEFAULT '',         #PIR报警等级
  `pir_ineffective_time` varchar(2000) DEFAULT '',  #PIR不生效时间配置项
  `current_wifi` varchar(50) DEFAULT '',            #当前WIFI
  `sub_category` varchar(50) DEFAULT '',            #门铃类别
  `disturb_mode` varchar(50) DEFAULT '',            #免打扰模式
  `otherproperty` varchar(1000) DEFAULT '',         #其他属性
  `createdate` datetime NOT NULL,
  `updatedate` datetime DEFAULT CURRENT_TIMESTAMP,
  `status` int(11) DEFAULT '0',
  `extend` varchar(4000) DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE KEY (deviceid)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `t_device_event_info`;
CREATE TABLE `t_device_event_info` (
  `id` varchar(36) NOT NULL,
  `eventid` varchar(32) NOT NULL,
  `deviceid` varchar(32) NOT NULL,
  `devicetype` int(11) DEFAULT '-1',
  `eventtype` int(11) NOT NULL,     #事件类型，支持Motion、Ring、LowPower
  `eventstate` int(11),             #时间状态，Motion支持Motion、Answered_Motion，Ring支持Accepted_Ring、Missed_Ring、Message_Ring
  `readstate` int(11) NOT NULL,     #查看状态，包括Unread、Read
  `fileid` varchar(200) DEFAULT '',
  `thumbnail` varchar(1000) DEFAULT '',
  `storedtime` int(11) NOT NULL DEFAULT '0',  #已存储时长
  `createdate` datetime NOT NULL,
  `status` int(11) DEFAULT '0',
  `extend` varchar(4000) DEFAULT '',
  PRIMARY KEY (`id`),
  INDEX index_ref1(eventid),
  INDEX index_ref2(deviceid)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

