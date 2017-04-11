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
  `createdate` datetime NOT NULL,
  `status` int(11) NOT NULL DEFAULT '0', #0正常，1删除
  `extend` varchar(4000) DEFAULT NULL,
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
  `latestversion` varchar(20), #最新版本
  `description` varchar(500), #版本详情描述
  `forceversion` varchar(20), #强制升级的版本
  `filename` varchar(100),
  `fileid` varchar(256),
  `filesize` int(11),
  `filepath` varchar(256),
  `leaseduration` int(11),
  `updatedate` datetime NOT NULL,
  `status` int(11) NOT NULL DEFAULT '0', #0正常，1删除
  `extend` varchar(4000) DEFAULT NULL,
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

