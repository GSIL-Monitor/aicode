CREATE DATABASE IF NOT EXISTS PlatformDB DEFAULT CHARSET utf8 COLLATE utf8_general_ci;

use PlatformDB;

DROP TABLE IF EXISTS `t_ip_country`;
CREATE TABLE `t_ip_country` (
    `id` VARCHAR(36) NOT NULL,
    `ip` VARCHAR(46) NOT NULL,
    `countrycode` VARCHAR(8) NOT NULL,
    `createtime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `updatetime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `status` INT(11) NOT NULL DEFAULT '0',
    `visitecount` INT(11) NOT NULL DEFAULT '0',
    `extend` VARCHAR(4000) NULL DEFAULT NULL,
    PRIMARY KEY (`id`),
    UNIQUE KEY  `ukeyip` (`ip`),
    INDEX `ip` (`ip`),
    INDEX `createtime` (`createtime`),
    INDEX `updatetime` (`updatetime`),
    INDEX `status` (`status`),
    INDEX `visitecount` (`visitecount`)
)
ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COMMENT='ip与地区对应表';

DROP TABLE IF EXISTS `t_timezone_info`;
CREATE TABLE `t_timezone_info` (
    `id` VARCHAR(50) NOT NULL,
    `countrycode` VARCHAR(50) NOT NULL,
    `country_en` VARCHAR(50) NULL DEFAULT NULL,
    `country_cn` VARCHAR(50) NULL DEFAULT NULL,
    `countryphone` VARCHAR(50) NULL DEFAULT NULL,
    `countrySC` VARCHAR(50) NULL DEFAULT NULL,
    `countrySQ` VARCHAR(50) NULL DEFAULT NULL,
    `createtime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `updatetime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `status` INT(11) NOT NULL DEFAULT '0',
    `extend` VARCHAR(1000) NOT NULL DEFAULT '0',
    PRIMARY KEY (`id`),
    INDEX `countrycode` (`countrycode`)
)
ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COMMENT='时区信息表';

DROP TABLE IF EXISTS `t_p2pserver_info`;
CREATE TABLE `t_p2pserver_info` (
    `id` VARCHAR(36) NOT NULL,
    `serverip` VARCHAR(46) NOT NULL COMMENT '服务器ip',
    `port` INT(11) NOT NULL COMMENT '服务器端口',
    `domain` VARCHAR(50) NOT NULL COMMENT '服务器域名',
    `countrycode` VARCHAR(50) NOT NULL COMMENT '国家代码',
    `cluster` VARCHAR(50) NOT NULL COMMENT '所属服务器集群编号（用于分配p2pid）',
    `flag` VARCHAR(50) NOT NULL COMMENT 'P2P技术提供者标志',
    `connectparams` VARCHAR(2000) NOT NULL COMMENT 'P2P连接参数，以|分隔',
    `createtime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `updatetime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `status` INT(11) NOT NULL DEFAULT '0',
    `extend` VARCHAR(4000) NULL DEFAULT NULL,
    PRIMARY KEY (`id`),
    INDEX `serverip` (`serverip`),
    INDEX `flag` (`flag`)
)
ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COMMENT='p2p服务器信息';

DROP TABLE IF EXISTS `t_p2pid_sy`;
CREATE TABLE `t_p2pid_sy` (
    `id` VARCHAR(36) NOT NULL,
    `p2pid` VARCHAR(50) NOT NULL,
    `param1` VARCHAR(100) NOT NULL COMMENT 'license值',
    `param2` VARCHAR(100) NOT NULL COMMENT 'pushid',
    `deviceid` VARCHAR(100) NULL DEFAULT NULL,
    `cluster` VARCHAR(100) NOT NULL COMMENT '所属服务器集群',
    `validity_period` INT(11) NOT NULL DEFAULT '0' COMMENT '有效期，单位：秒，0代表永久有效',
    `createtime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `updatetime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `status` INT(11) NOT NULL DEFAULT '0',
    `extend` VARCHAR(4000) NULL DEFAULT NULL,
    PRIMARY KEY (`id`),
    INDEX `p2pid` (`p2pid`),
    INDEX `createtime` (`createtime`),
    INDEX `updatetime` (`updatetime`),
    INDEX `status` (`status`),
    INDEX `license` (`param1`)
)
ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COMMENT='尚云P2Pid库';

#TUTK的P2PID会少量使用，存放在浪涛P2PID表中
DROP TABLE IF EXISTS `t_p2pid_lt`;
CREATE TABLE `t_p2pid_lt` (
    `id` VARCHAR(36) NOT NULL,
    `buildin` INT(11) NOT NULL, #P2P分配方式，0-动态分配，1-设备烧录
    `supplier` VARCHAR(50) NOT NULL, #供应商
    `p2pid` VARCHAR(50) NOT NULL,
    `deviceid` VARCHAR(100) NULL DEFAULT NULL,
    `validity_period` INT(11) NOT NULL DEFAULT '0' COMMENT '有效期，单位：秒，0代表永久有效',
    `createtime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `updatetime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `status` INT(11) NOT NULL DEFAULT '0',
    `extend` VARCHAR(4000) NULL DEFAULT NULL,
    PRIMARY KEY (`id`),
    INDEX `p2pid` (`p2pid`),
    INDEX `updatetime` (`updatetime`),
    INDEX `status` (`status`)
)
ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COMMENT='浪涛P2Pid库';



