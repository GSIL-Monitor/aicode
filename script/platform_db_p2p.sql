CREATE DATABASE IF NOT EXISTS PlatformDB DEFAULT CHARSET utf8 COLLATE utf8_general_ci;

use PlatformDB;

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
    UNIQUE KEY (`ip`),
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
    `createtime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `updatetime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
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
    `validity_period` INT(11) NOT NULL DEFAULT '0' COMMENT '有效期，单位：秒，0代表永久有效',
    `createtime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `updatetime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
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



