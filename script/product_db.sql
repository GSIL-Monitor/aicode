CREATE DATABASE IF NOT EXISTS ProductDB DEFAULT CHARSET utf8 COLLATE utf8_general_ci;

use ProductDB;

DROP TABLE IF EXISTS `t_sale_company_info`;
CREATE TABLE `t_sale_company_info` (
  `id` varchar(36) NOT NULL,
  `scid` varchar(100) NOT NULL                       COMMENT '销售公司ID',
  `scname` varchar(100) NOT NULL                     COMMENT '销售公司名称',
  `email` varchar(200) NOT NULL                      COMMENT '邮箱',
  `phone` varchar(2000) DEFAULT ''                   COMMENT '联系电话',
  `back_receiver` varchar(500) DEFAULT ''            COMMENT '退货联系人',
  `back_phone` varchar(2000) DEFAULT ''              COMMENT '退货联系电话',
  `status` int(11) NOT NULL DEFAULT '0'              COMMENT '0正常，1删除',
  `extend` varchar(2000) DEFAULT ''                  COMMENT '扩展信息',
  PRIMARY KEY (`id`),
  INDEX index_ref1(scid)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


DROP TABLE IF EXISTS `t_product_info`;
CREATE TABLE `t_product_info` (
  `id` varchar(36) NOT NULL,
  `pdtid` varchar(100) NOT NULL                      COMMENT '产品ID',
  `pdtname` varchar(200) NOT NULL                    COMMENT '产品名称',
  `typeinfo` int(11) NOT NULL DEFAULT '0'            COMMENT '产品类型',
  `aliasname` varchar(200) DEFAULT ''                COMMENT '产品别名',
  `pdtprice` DOUBLE NOT NULL DEFAULT '0'             COMMENT '产品单价',
  `pic` varchar(300) DEFAULT ''                      COMMENT '产品图片http地址',
  `status` int(11) NOT NULL DEFAULT '0'              COMMENT '0正常，1删除',
  `extend` varchar(2000) DEFAULT ''                  COMMENT '扩展信息',
  PRIMARY KEY (`id`),
  INDEX index_ref1(pdtid),
  INDEX index_ref2(typeinfo)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;


DROP TABLE IF EXISTS `t_product_property`;
CREATE TABLE `t_product_property` (
  `id` varchar(36) NOT NULL,
  `pdtpptid` varchar(100) NOT NULL                   COMMENT '产品属性ID',
  `pdtid` varchar(100) NOT NULL                      COMMENT '产品ID',
  `property_type` int(11) NOT NULL DEFAULT '0'       COMMENT '属性类型，0文本属性，1价格属性，2图片属性，3视频属性',
  `property_name` varchar(200) DEFAULT ''            COMMENT '属性名称',
  `property_value` varchar(2000) DEFAULT ''          COMMENT '属性值',
  `status` int(11) NOT NULL DEFAULT '0'              COMMENT '0正常，1删除',
  `extend` varchar(2000) DEFAULT ''                  COMMENT '扩展信息',
  PRIMARY KEY (`id`),
  INDEX index_ref1(pdtpptid),
  INDEX index_ref2(pdtid)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `t_order_info`;
CREATE TABLE `t_order_info` (
  `id` varchar(36) NOT NULL,
  `ordid` varchar(100) NOT NULL                      COMMENT '订单ID，格式：20181028-153020-uuid',
  `ordname` varchar(100) NOT NULL                    COMMENT '订单名称',
  `typeinfo` int(11) NOT NULL DEFAULT '0'            COMMENT '订单类型，0普通，1加急',
  `userid` varchar(100) NOT NULL DEFAULT ''          COMMENT '用户ID',
  `totalprice` DOUBLE NOT NULL DEFAULT '0'           COMMENT '订单总金额',
  `ordstatus` int(11) NOT NULL DEFAULT '0'           COMMENT '0待支付，1已支付，2退款，3已发货，4关闭',
  `express_info` varchar(2000) DEFAULT ''            COMMENT '快递单信息，json字符串，格式：["xxxx", "mmmm", "yyyy"]',
  `history_ordstatus` varchar(2000) DEFAULT ''       COMMENT '历史订单状态，json字符串，格式：["0", "1", "2", "3"]',
  `address` varchar(2000) DEFAULT ''                 COMMENT '订单地址',
  `receiver` varchar(500) DEFAULT ''                 COMMENT '收货人',
  `phone` varchar(2000) DEFAULT ''                   COMMENT '联系电话',
  `back_receiver` varchar(500) DEFAULT ''            COMMENT '退货联系人',
  `back_phone` varchar(2000) DEFAULT ''              COMMENT '退货联系电话',
  `back_express_info` varchar(2000) DEFAULT ''       COMMENT '退货快递单信息，json字符串，格式：["xxxx", "mmmm", "yyyy"]',
  `create_date` datetime default current_timestamp   COMMENT '创建日期',
  `status` int(11) NOT NULL DEFAULT '0'              COMMENT '0正常，1删除',
  `extend` varchar(2000) DEFAULT ''                  COMMENT '扩展信息',
  PRIMARY KEY (`id`),
  INDEX index_ref1(ordid),
  INDEX index_ref2(ordname)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `t_order_detail`;
CREATE TABLE `t_order_detail` (
  `id` varchar(36) NOT NULL,
  `orddtid` varchar(100) NOT NULL                    COMMENT '订单详情ID',
  `ordid` varchar(100) NOT NULL                      COMMENT '订单ID，格式：20181028-153020-uuid',
  `pdtid` varchar(100) NOT NULL                      COMMENT '产品ID',
  `pdtnumber` int(11) NOT NULL DEFAULT '0'           COMMENT '产品数量',
  `pdtprice` DOUBLE NOT NULL DEFAULT '0'             COMMENT '产品单价',
  `totalprice` DOUBLE NOT NULL DEFAULT '0'           COMMENT '产品总金额，允许自定义金额',
  `status` int(11) NOT NULL DEFAULT '0'              COMMENT '0正常，1删除',
  `extend` varchar(2000) DEFAULT ''                  COMMENT '扩展信息',
  PRIMARY KEY (`id`),
  INDEX index_ref1(ordid),
  INDEX index_ref2(orddtid)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8;

