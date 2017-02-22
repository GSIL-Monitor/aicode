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
  `relation` int(11) NOT NULL DEFAULT '0', #关系包括，拥有0、被分享1、分享中2、转移3，目前只用0、1、2
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

CREATE TABLE `t_ip_country` (
	`id` VARCHAR(36) NOT NULL,
	`ip` VARCHAR(46) NOT NULL,
	`countrycode` VARCHAR(8) NOT NULL,
	`createtime` DATETIME NOT NULL,
	`updatetime` DATETIME NOT NULL,
	`status` INT(11) NOT NULL DEFAULT '0',
	`visitecount` INT(11) NOT NULL DEFAULT '0',
	`extend` VARCHAR(4000) NULL DEFAULT NULL,
	PRIMARY KEY (`id`),
	INDEX `ip` (`ip`),
	INDEX `createtime` (`createtime`),
	INDEX `updatetime` (`updatetime`),
	INDEX `status` (`status`),
	INDEX `visitecount` (`visitecount`)
)
COMMENT='ip与地区对应表'
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;

CREATE TABLE `t_timezone_info` (
	`id` VARCHAR(36) NOT NULL,
	`countrycode` VARCHAR(8) NOT NULL COMMENT '国家代码',
	`country_en` VARCHAR(50) NOT NULL COMMENT '国家英文名称',
	`country_cn` VARCHAR(50) NOT NULL COMMENT '国家中文名称',
	`countryphone` VARCHAR(8) NOT NULL COMMENT '国家电话号码前缀',
	`countrySC` INT(11) NOT NULL,
	`countrySQ` INT(11) NOT NULL,
	`createtime` DATETIME NOT NULL,
	`updatetime` DATETIME NOT NULL,
	`status` INT(11) NOT NULL DEFAULT '0',
	`extend` VARCHAR(4000) NULL DEFAULT NULL,
	PRIMARY KEY (`id`),
	INDEX `countrycode` (`countrycode`),
	INDEX `country_en` (`country_en`),
	INDEX `createtime` (`createtime`),
	INDEX `updatetime` (`updatetime`),
	INDEX `status` (`status`)
)
COMMENT='时区信息表'
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;

CREATE TABLE `t_p2pserver_info` (
	`id` VARCHAR(36) NOT NULL,
	`serverip` VARCHAR(46) NOT NULL COMMENT '服务器ip',
	`port` INT(11) NOT NULL COMMENT '服务器端口',
	`domain` VARCHAR(50) NOT NULL COMMENT '服务器域名',
	`countrycode` VARCHAR(50) NOT NULL COMMENT '国家代码',
	`cluster` VARCHAR(50) NOT NULL COMMENT '所属服务器集群编号（用于分配p2pid）',
	`flag` VARCHAR(50) NOT NULL COMMENT 'P2P技术提供者标志',
	`connectparams` VARCHAR(2000) NOT NULL COMMENT 'P2P连接参数，以|分隔',
	`createtime` DATETIME NOT NULL,
	`updatetime` DATETIME NOT NULL,
	`status` INT(11) NOT NULL DEFAULT '0',
	`extend` VARCHAR(4000) NULL DEFAULT NULL,
	PRIMARY KEY (`id`),
	INDEX `serverip` (`serverip`),
	INDEX `flag` (`flag`)
)
COMMENT='p2p服务器信息'
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;

CREATE TABLE `t_p2pid_sy` (
	`id` VARCHAR(36) NOT NULL,
	`p2pid` VARCHAR(50) NOT NULL,
	`deviceid` VARCHAR(100) NULL DEFAULT NULL,
	`cluster` VARCHAR(100) NOT NULL COMMENT '所属服务器集群',
	`createtime` DATETIME NOT NULL,
	`updatetime` DATETIME NOT NULL,
	`status` INT(11) NOT NULL DEFAULT '0',
	`extend` VARCHAR(4000) NULL DEFAULT NULL,
	PRIMARY KEY (`id`),
	INDEX `p2pid` (`p2pid`),
	INDEX `createtime` (`createtime`),
	INDEX `updatetime` (`updatetime`),
	INDEX `status` (`status`)
)
COMMENT='尚云P2Pid库'
COLLATE='utf8_general_ci'
ENGINE=InnoDB
;



