CREATE
	DEFINER = 'root'@'%'
EVENT CustomerFlowDB.e_customer_flow_statistic_hourly
	ON SCHEDULE EVERY '1' HOUR
	STARTS '2017-08-24 17:05:00'
	COMMENT '生成客流信息时统计表'
	DO
BEGIN
    set @post_hour = DATE_FORMAT(DATE_SUB(now(), interval 1 HOUR), '%Y-%m-%d %H:');
    CALL call_customer_flow_statistic_hourly(@post_hour);
END;

ALTER EVENT CustomerFlowDB.e_customer_flow_statistic_hourly
	ENABLE
