CREATE DATABASE IF NOT EXISTS PlatformDB DEFAULT CHARSET utf8 COLLATE utf8_general_ci;

use PlatformDB;

DROP TABLE IF EXISTS `t_user_info`;
CREATE TABLE `t_user_info` (
  `id` varchar(36) NOT NULL,
  `userid` varchar(100) NOT NULL,
  `username` varchar(100) NOT NULL,
  `userpassword` varchar(100) DEFAULT NULL,
  `typeinfo` int(11) NOT NULL DEFAULT '0',   
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
  `srcuserid` varchar(100) NOT NULL,       #设备所有者的用户ID
  `relation` int(11) NOT NULL DEFAULT '0', #关系包括，拥有0、被分享1、分享中2、转移3，目前只用0、1、2
  `begindate` datetime NOT NULL,
  `enddate` datetime NOT NULL,
  `createdate` datetime NOT NULL,
  `status` int(11) NOT NULL DEFAULT '0',
  `extend` varchar(4000) DEFAULT NULL,
  PRIMARY KEY (`id`),
  INDEX index_ref1(userid, srcuserid),
  INDEX index_ref2(deviceid),
  INDEX index_ref3(userid, createdate),
  INDEX index_ref4(status),
  INDEX index_ref5(createdate)
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





