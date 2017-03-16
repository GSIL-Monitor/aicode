CREATE DATABASE IF NOT EXISTS ManagementDB DEFAULT CHARSET utf8 COLLATE utf8_general_ci;

use ManagementDB;

DROP TABLE IF EXISTS `t_user_online_status_info`;
CREATE TABLE `t_user_online_status_info` (
  `id` varchar(36) NOT NULL,
  `sequence` bigint NOT NULL, #用户登录会话ID
  `userid` varchar(100) NOT NULL,
  `username` varchar(100) NOT NULL,
  `useraliasname` varchar(100),
  `clusterid` varchar(100) NOT NULL, #集群ID
  `logintime` datetime,
  `logouttime` datetime,
  `clienttype` int(11), #客户终端类型
  `createdate` datetime NOT NULL,
  `status` int(11) NOT NULL DEFAULT 0, #0正常，1删除
  PRIMARY KEY (`id`),
  INDEX index_ref1(userid),
  INDEX index_ref2(clusterid),
  INDEX index_ref3(logintime, logouttime)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `t_device_online_status_info`;
CREATE TABLE `t_device_online_status_info` (
  `id` varchar(36) NOT NULL,
  `sequence` bigint NOT NULL, #设备登录会话ID
  `deviceid` varchar(100) NOT NULL,
  `devicename` varchar(100) NOT NULL,
  `clusterid` varchar(100) NOT NULL, #集群ID
  `logintime` datetime,
  `logouttime` datetime,
  `devicetype` int(11), #设备类型
  `createdate` datetime NOT NULL,
  `status` int(11) NOT NULL DEFAULT 0, #0正常，1删除
  PRIMARY KEY (`id`),
  INDEX index_ref1(deviceid),
  INDEX index_ref2(clusterid),
  INDEX index_ref3(logintime, logouttime)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `t_cluster_info`;
CREATE TABLE `t_cluster_info` (
  `id` varchar(36) NOT NULL,
  `clusterid` varchar(100) NOT NULL,
  `clusteraddress` varchar(256) NOT NULL, #集群地址
  `managementaddress` varchar(256), #管理中心地址
  `aliasname` varchar(100),
  `createdate` datetime NOT NULL,
  `status` int(11) NOT NULL DEFAULT 0, #0正常，1删除
  PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;
