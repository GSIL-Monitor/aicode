create database if not exists CustomerFlowDB default charset utf8 collate utf8_general_ci;

use CustomerFlowDB;

#drop table if exists `t_user_info`;
#create table `t_user_info` (
#    `id` varchar(36) not null,
#    `user_id` varchar(36) not null                   comment '用户id',
#    `user_name` varchar(64) not null                 comment '用户名',
#    `password` varchar(32) not null                  comment '密码',
#    `nickname` varchar(64) default ''                comment '昵称',
#    `email` varchar(64) not null                     comment '邮箱',
#    `app_id` varchar(32) not null                    comment '程序id',
#    `state` int default 0                            comment '0-正常，1-删除',
#    `create_date` datetime not null                  comment '创建日期',
#    `update_date` datetime default current_timestamp comment '更新日期',
#    primary key (`id`),
#    index index_ref1 (`user_id`),
#    index index_ref2 (`user_name`)
#) engine=innodb auto_increment=1 default charset=utf8 comment='用户信息表';
#
#drop table if exists `t_device_info`;
#create table 't_device_info' (
#    `id` varchar(36) not null,
#    `device_id` varchar(36) not null                 comment '设备id',
#    `user_id` varchar(36) not null                   comment '用户id',
#    `store_id` varchar(36) not null                  comment '店铺id',
#    `device_name` varchar(64) default ''             comment '设备名',
#    `password` varchar(32) default ''                comment '密码',
#    `device_type` varchar(32) not null               comment '设备类型',
#    `state` int default 0                            comment '0-正常，1-删除',
#    `create_date` datetime not null                  comment '创建日期',
#    `update_date` datetime default current_timestamp comment '更新日期',
#    primary key (`id`),
#    index index_ref1 (`device_id`),
#    index index_ref2 (`user_id`)
#) engine=innodb auto_increment=1 default charset=utf8 comment='设备信息表';

drop table if exists `t_company_info`;
create table `t_company_info` (
    `id` varchar(36) not null,
    `company_id` varchar(36) not null                comment '公司ID',
    `company_name` varchar(256) not null             comment '公司名称',
    `industry` varchar(256) not null                 comment '行业',
    `address` varchar(512) not null                  comment '地址',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`company_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='公司信息表';

drop table if exists `t_company_user_info`;
create table `t_company_user_info` (
    `id` varchar(36) not null,
    `company_id` varchar(36) not null                comment '公司ID',
    `user_id` varchar(36) not null                   comment '用户ID',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`company_id`),
    index index_ref2 (`user_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='公司用户信息表';

drop table if exists `t_group_info`;
create table `t_group_info` (
    `id` varchar(36) not null,
    `group_id` varchar(36) not null                  comment '群组ID',
    `group_name` varchar(64) not null                comment '群组名',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`group_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='群组信息表';

drop table if exists `t_area_info`;
create table `t_area_info` (
    `id` varchar(36) not null,
    `area_id` varchar(36) not null                   comment '区域ID',
    `area_name` varchar(256) not null                comment '区域名',
    `level` int not null                             comment '区域层级',
    `parent_area_id` varchar(36) default ''          comment '父区域ID',
    `company_id` varchar(36) not null                comment '公司ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    `extend` varchar(4000) default ''                comment '扩展',
    primary key (`id`),
    index index_ref1 (`area_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='区域信息表';

drop table if exists `t_store_info`;
create table `t_store_info` (
    `id` varchar(36) not null,
    `store_id` varchar(36) not null                  comment '店铺ID',
    `store_name` varchar(64) not null                comment '店铺名',
    `goods_category` varchar(128) default ''         comment '经营品类',
    `address` varchar(256) default ''                comment '地址',
    `area_id` varchar(36) default ''                 comment '区域ID',
    `open_state` int default 1                       comment '营业状态',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    `extend` varchar(4000) default ''                comment '扩展',
    primary key (`id`),
    index index_ref1 (`store_id`),
    index index_ref2 (`store_name`)
) engine=innodb auto_increment=1 default charset=utf8 comment='店铺信息表';

drop table if exists `t_entrance_info`;
create table `t_entrance_info` (
    `id` varchar(36) not null,
    `entrance_id` varchar(36) not null               comment '出入口ID',
    `store_id` varchar(36) not null                  comment '店铺ID',
    `entrance_name` varchar(64) not null             comment '出入口名',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    unique key (`store_id`, `entrance_name`),
    index index_ref1 (`entrance_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='出入口信息表';

drop table if exists `t_evaluation_item`;
create table `t_evaluation_item` (
    `id` varchar(36) not null,
    `item_id` varchar(36) not null                   comment '条目ID',
    `item_name` varchar(256) not null                comment '条目名称',
    `description` varchar(1024) default ''            comment '描述',
    `total_point` double not null                    comment '总分',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`item_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='店铺考评项目表';

drop table if exists `t_store_evaluation`;
create table `t_store_evaluation` (
    `id` varchar(36) not null,
    `evaluation_id` varchar(36) not null             comment '考评ID',
    `store_id` varchar(36) not null                  comment '店铺ID',
    `user_id_create` varchar(36) not null            comment '创建者用户ID',
    `user_id_check` varchar(36) not null             comment '审查人用户ID',
    `check_status` int not null                      comment '审查状态',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`evaluation_id`),
    index index_ref2 (`store_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='店铺考评表';

drop table if exists `t_store_evaluation_score`;
create table `t_store_evaluation_score` (
    `id` varchar(36) not null,
    `evaluation_id` varchar(36) not null             comment '考评ID',
    `item_id` varchar(36) not null                   comment '考评条目',
    `score` double default 0.0                       comment '得分',
    `description` varchar(1024) default ''           comment '描述',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`evaluation_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='店铺考评得分表';

drop table if exists `t_store_deal_pos`;
create table `t_store_deal_pos` (
    `id` varchar(36) not null,
    `store_id` varchar(36) not null                  comment '店铺ID',
    `deal_date` datetime not null                    comment '交易日期',
    `order_amount` int default 0                     comment '交易笔数',
    `goods_amount` int default 0                     comment '商品件数',
    `deal_amount` double default 0.0                 comment '交易总额',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    unique key (`store_id`, `deal_date`)
) engine=innodb auto_increment=1 default charset=utf8 comment='店铺交易POS数据表';

drop table if exists `t_user_group_association`;
create table `t_user_group_association` (
    `id` varchar(36) not null,
    `user_id` varchar(36) not null                   comment '用户ID',
    `group_id` varchar(36) not null                  comment '群组ID',
    `user_role` varchar(32) not null                 comment '用户角色',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`user_id`),
    index index_ref2 (`group_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='用户-群组关联表';

drop table if exists `t_user_area_association`;
create table `t_user_area_association` (
    `id` varchar(36) not null,
    `user_id` varchar(36) not null                   comment '用户ID',
    `area_id` varchar(36) not null                   comment '区域ID',
    `user_role` varchar(32) not null                 comment '用户角色',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`user_id`),
    index index_ref2 (`area_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='用户-区域关联表';

drop table if exists `t_user_store_association`;
create table `t_user_store_association` (
    `id` varchar(36) not null,
    `user_id` varchar(36) not null                   comment '用户ID',
    `store_id` varchar(36) not null                  comment '店铺ID',
    `user_role` varchar(32) not null                 comment '用户角色',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    unique key (`user_id`, `store_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='用户-店铺关联表';

drop table if exists `t_group_area_association`;
create table `t_group_area_association` (
    `id` varchar(36) not null,
    `group_id` varchar(36) not null                  comment '群组ID',
    `area_id` varchar(36) not null                   comment '区域ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`group_id`),
    index index_ref2 (`area_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='群组-区域关联表';

drop table if exists `t_store_device_association`;
create table `t_store_device_association` (
    `id` varchar(36) not null,
    `store_id` varchar(36) not null                  comment '店铺ID',
    `device_id` varchar(36) not null                 comment '设备ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    unique key (`store_id`, `device_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='店铺-设备关联表';

drop table if exists `t_store_group_association`;
create table `t_store_group_association` (
    `id` varchar(36) not null,
    `store_id` varchar(36) not null                  comment '店铺ID',
    `group_id` varchar(36) not null                  comment '群组ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`store_id`),
    index index_ref2 (`group_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='店铺-群组关联表';

drop table if exists `t_store_area_association`;
create table `t_store_area_association` (
    `id` varchar(36) not null,
    `store_id` varchar(36) not null                  comment '店铺ID',
    `area_id` varchar(36) not null                   comment '区域ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`store_id`),
    index index_ref2 (`area_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='店铺-区域关联表';

drop table if exists `t_entrance_device_association`;
create table `t_entrance_device_association` (
    `id` varchar(36) not null,
    `entrance_id` varchar(36) not null               comment '出入口ID',
    `device_id` varchar(36) not null                 comment '设备ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    unique key (`entrance_id`, `device_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='出入口-设备关联表';

drop table if exists `t_customer_flow_original_1`;
create table `t_customer_flow_original_1` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    index index_ref1 (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息原始数据表-1月';

drop table if exists `t_customer_flow_original_2`;
create table `t_customer_flow_original_2` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    index index_ref1 (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息原始数据表-2月';

drop table if exists `t_customer_flow_original_3`;
create table `t_customer_flow_original_3` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    index index_ref1 (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息原始数据表-3月';

drop table if exists `t_customer_flow_original_4`;
create table `t_customer_flow_original_4` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    index index_ref1 (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息原始数据表-4月';

drop table if exists `t_customer_flow_original_5`;
create table `t_customer_flow_original_5` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    index index_ref1 (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息原始数据表-5月';

drop table if exists `t_customer_flow_original_6`;
create table `t_customer_flow_original_6` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    index index_ref1 (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息原始数据表-6月';

drop table if exists `t_customer_flow_original_7`;
create table `t_customer_flow_original_7` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    index index_ref1 (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息原始数据表-7月';

drop table if exists `t_customer_flow_original_8`;
create table `t_customer_flow_original_8` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    index index_ref1 (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息原始数据表-8月';

drop table if exists `t_customer_flow_original_9`;
create table `t_customer_flow_original_9` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    index index_ref1 (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息原始数据表-9月';

drop table if exists `t_customer_flow_original_10`;
create table `t_customer_flow_original_10` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    index index_ref1 (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息原始数据表-10月';

drop table if exists `t_customer_flow_original_11`;
create table `t_customer_flow_original_11` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    index index_ref1 (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息原始数据表-11月';

drop table if exists `t_customer_flow_original_12`;
create table `t_customer_flow_original_12` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    index index_ref1 (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息原始数据表-12月';

drop table if exists `t_customer_flow_statistic_half_hourly_1`;
create table `t_customer_flow_statistic_half_hourly_1` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    unique key (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息半小时统计表-第一季度';

drop table if exists `t_customer_flow_statistic_half_hourly_2`;
create table `t_customer_flow_statistic_half_hourly_2` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    unique key (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息半小时统计表-第二季度';

drop table if exists `t_customer_flow_statistic_half_hourly_3`;
create table `t_customer_flow_statistic_half_hourly_3` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    unique key (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息半小时统计表-第三季度';

drop table if exists `t_customer_flow_statistic_half_hourly_4`;
create table `t_customer_flow_statistic_half_hourly_4` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    unique key (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息半小时统计表-第四季度';

drop table if exists `t_customer_flow_statistic_hourly_1`;
create table `t_customer_flow_statistic_hourly_1` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    unique key (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息时统计表-第一季度';

drop table if exists `t_customer_flow_statistic_hourly_2`;
create table `t_customer_flow_statistic_hourly_2` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    unique key (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息时统计表-第二季度';

drop table if exists `t_customer_flow_statistic_hourly_3`;
create table `t_customer_flow_statistic_hourly_3` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    unique key (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息时统计表-第三季度';

drop table if exists `t_customer_flow_statistic_hourly_4`;
create table `t_customer_flow_statistic_hourly_4` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    unique key (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息时统计表-第四季度';

drop table if exists `t_customer_flow_statistic_daily`;
create table `t_customer_flow_statistic_daily` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    unique key (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息日统计表';

drop table if exists `t_customer_flow_statistic_monthly`;
create table `t_customer_flow_statistic_monthly` (
    `id` varchar(36) not null,
    `device_id` varchar(36) not null                 comment '设备ID',
    `data_time` datetime not null                    comment '数据时间',
    `enter_number` int default 0                     comment '进入人数',
    `leave_number` int default 0                     comment '离开人数',
    `stay_number` int default 0                      comment '滞留人数',
    `create_date` datetime default current_timestamp comment '创建日期',
    primary key (`id`),
    index index_ref1 (`device_id`, `data_time`)
) engine=innodb auto_increment=1 default charset=utf8 comment='客流信息月统计表';

drop table if exists `t_role_info`;
create table `t_role_info` (
    `id` varchar(36) not null,
    `role_id` varchar(36) not null                   comment '角色ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`role_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='角色信息表';

drop table if exists `t_menu_info`;
create table `t_menu_info` (
    `id` varchar(36) not null,
    `menu_id` int not null                           comment '菜单ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`menu_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='菜单信息表';

drop table if exists `t_user_role_association`;
create table `t_user_role_association` (
    `id` varchar(36) not null,
    `user_id` varchar(36) not null                   comment '用户ID',
    `role_id` varchar(36) not null                   comment '角色ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`user_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='用户角色表';

drop table if exists `t_role_menu_association`;
create table `t_role_menu_association` (
    `id` varchar(36) not null,
    `role_id` varchar(36) not null                   comment '角色ID',
    `menu_id` varchar(36) not null                   comment '菜单ID',
    `access_permission` varchar(16) not null         comment '访问权限，allow-允许访问，disallow-不允许访问',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`role_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='角色菜单表';

drop table if exists `t_regular_patrol_plan`;
create table `t_regular_patrol_plan` (
    `id` varchar(36) not null,
    `plan_id` varchar(36) not null                   comment '计划ID',
    `user_id` varchar(36) not null                   comment '用户ID',
    `plan_name` varchar(256) not null                comment '计划名称',
    `enable` varchar(16) not null                    comment '是否启用',
    `last_update_time` datetime not null             comment '上次更新时间',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`plan_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='定时巡店计划表';

drop table if exists `t_patrol_plan_store_association`;
create table `t_patrol_plan_store_association` (
    `id` varchar(36) not null,
    `plan_id` varchar(36) not null                   comment '计划ID',
    `store_id` varchar(36) not null                  comment '店铺ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`plan_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='定时巡店计划-店铺关联表';

drop table if exists `t_patrol_plan_entrance_association`;
create table `t_patrol_plan_entrance_association` (
    `id` varchar(36) not null,
    `plan_id` varchar(36) not null                   comment '计划ID',
    `entrance_id` varchar(36) not null               comment '出入口ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`plan_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='定时巡店计划-出入口关联表';

drop table if exists `t_regular_patrol_time`;
create table `t_regular_patrol_time` (
    `id` varchar(36) not null,
    `plan_id` varchar(36) not null                   comment '计划ID',
    `patrol_time` varchar(256) not null              comment '巡查时间',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`plan_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='定时巡店时间表';

drop table if exists `t_patrol_plan_entrance_association`;
create table `t_patrol_plan_entrance_association` (
    `id` varchar(36) not null,
    `plan_id` varchar(36) not null                   comment '计划ID',
    `entrance_id` varchar(36) not null               comment '出入口ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`plan_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='定时巡店计划-出入口关联表';

drop table if exists `t_patrol_plan_user_association`;
create table `t_patrol_plan_user_association` (
    `id` varchar(36) not null,
    `plan_id` varchar(36) not null                   comment '计划ID',
    `user_id` varchar(36) not null                   comment '用户ID',
    `user_role` varchar(32) not null                 comment '用户角色',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`plan_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='定时巡店计划-用户关联表';

drop table if exists `t_smart_guard_store_plan`;
create table `t_smart_guard_store_plan` (
    `id` varchar(36) not null,
    `plan_id` varchar(36) not null                   comment '计划ID',
    `user_id` varchar(36) not null                   comment '用户ID',
    `store_id` varchar(36) not null                  comment '店铺ID',
    `plan_name` varchar(256) not null                comment '计划名称',
    `enable` varchar(16) not null                    comment '是否启用',
    `begin_time` time not null                       comment '开始时间',
    `end_time` time not null                         comment '结束时间',
    `begin_time2` time default null                  comment '开始时间2',
    `end_time2` time default null                    comment '结束时间2',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`plan_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='智能看店计划表';

drop table if exists `t_guard_plan_store_association`;
create table `t_guard_plan_store_association` (
    `id` varchar(36) not null,
    `plan_id` varchar(36) not null                   comment '计划ID',
    `store_id` varchar(36) not null                  comment '店铺ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`plan_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='智能看店计划-店铺关联表';

drop table if exists `t_guard_plan_entrance_association`;
create table `t_guard_plan_entrance_association` (
    `id` varchar(36) not null,
    `plan_id` varchar(36) not null                   comment '计划ID',
    `entrance_id` varchar(36) not null               comment '出入口ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`plan_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='智能看店计划-出入口关联表';

drop table if exists `t_event_info`;
create table `t_event_info` (
    `id` varchar(36) not null,
    `event_id` varchar(36) not null                  comment '事件ID',
    `source` varchar(256) not null                   comment '事件来源',
    `submit_date` datetime not null                  comment '提交时间',
    `expire_date` datetime not null                  comment '过期时间',
    `user_id` varchar(36) default ''                 comment '用户ID',
    `device_id` varchar(36) default ''               comment '设备ID',
    `process_state` varchar(36) not null             comment '事件处理状态',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`event_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='事件信息表';

drop table if exists `t_event_type`;
create table `t_event_type` (
    `id` varchar(36) not null,
    `event_id` varchar(36) not null                  comment '事件ID',
    `event_type` int not null                        comment '事件类型',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`event_id`),
    index index_ref2 (`event_type`)
) engine=innodb auto_increment=1 default charset=utf8 comment='事件类型表';

drop table if exists `t_event_user_association`;
create table `t_event_user_association` (
    `id` varchar(36) not null,
    `event_id` varchar(36) not null                  comment '事件ID',
    `user_id` varchar(36) not null                   comment '用户ID',
    `user_role` varchar(32) not null                 comment '用户角色',
    `read_state` varchar(16) not null                comment '已读/未读，read/unread',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`event_id`),
    index index_ref2 (`user_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='事件责任人表';

drop table if exists `t_event_remark`;
create table `t_event_remark` (
    `id` varchar(36) not null,
    `event_id` varchar(36) not null                  comment '事件ID',
    `remark` varchar(1024) not null                  comment '备注信息',
    `user_id` varchar(36) not null                   comment '用户ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`event_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='事件备注表';

drop table if exists `t_vip_customer_info`;
create table `t_vip_customer_info` (
    `id` varchar(36) not null,
    `vip_id` varchar(36) not null                    comment 'VIP ID',
    `profile_picture` varchar(128) default ''        comment '头像图片路径',
    `vip_name` varchar(36) not null                  comment 'VIP姓名',
    `cellphone` varchar(16) not null                 comment '手机号',
    `visit_date` datetime not null                   comment '到访时间',
    `visit_times` int default 0                      comment '到访次数',
    `register_date` varchar(36) not null             comment '注册时间',
    `state` int default 0                            comment '0-正常，1-删除',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`vip_id`),
    index index_ref2 (`vip_name`)
) engine=innodb auto_increment=1 default charset=utf8 comment='VIP客户信息表';

drop table if exists `t_vip_consume_history`;
create table `t_vip_consume_history` (
    `id` varchar(36) not null,
    `consume_id` varchar(36) not null                comment '消费ID',
    `vip_id` varchar(36) not null                    comment 'VIP ID',
    `goods_name` varchar(128) not null               comment '商品名',
    `goods_number` int default 0                     comment '商品数量',
    `salesman` varchar(128) not null                 comment '业务员',
    `consume_amount` double default 0.0              comment '消费金额',
    `consume_date` datetime not null                 comment '消费日期',
    `state` int default 0                            comment '0-正常，1-删除',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`consume_id`),
    index index_ref2 (`vip_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='VIP客户消费记录表';

drop table if exists `t_remote_patrol_store`;
create table `t_remote_patrol_store` (
    `id` varchar(36) not null,
    `patrol_id` varchar(36) not null                 comment '巡查ID',
    `user_id` varchar(36) default ''                 comment '用户ID',
    `device_id` varchar(36) default ''               comment '设备ID',
    `entrance_id` varchar(36) not null               comment '出入口ID',
    `store_id` varchar(36) not null                  comment '店铺ID',
    `plan_id` varchar(36) default ''                 comment '巡店计划ID',
    `patrol_date` datetime not null                  comment '巡查日期',
    `patrol_result` int not null                     comment '巡查结果',
    `description` varchar(1024) default ''           comment '描述',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`patrol_id`),
    index index_ref2 (`user_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='远程巡店记录表';

drop table if exists `t_patrol_store_screenshot_association`;
create table `t_patrol_store_screenshot_association` (
    `id` varchar(36) not null,
    `patrol_id` varchar(36) not null                 comment '巡查ID',
    `screenshot_id` varchar(64) not null             comment '截图ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`patrol_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='远程巡店-截图关联表';

drop table if exists `t_user_client_id_association`;
create table `t_user_client_id_association` (
    `id` varchar(36) not null,
    `user_id` varchar(36) not null                   comment '用户ID',
    `client_id` varchar(36) not null                 comment '设备Client ID',
    `state` int default 0                            comment '0-正常，1-删除',
    `create_date` datetime not null                  comment '创建日期',
    `update_date` datetime default current_timestamp comment '更新日期',
    primary key (`id`),
    index index_ref1 (`user_id`)
) engine=innodb auto_increment=1 default charset=utf8 comment='用户-Client ID关联表';

