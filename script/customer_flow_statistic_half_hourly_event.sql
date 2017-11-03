CREATE
	DEFINER = 'root'@'%'
EVENT CustomerFlowDB.e_customer_flow_statistic_half_hourly
	ON SCHEDULE EVERY '30' MINUTE
	STARTS '2017-08-28 09:03:00'
	COMMENT '生成客流信息半小时统计表'
	DO
BEGIN
    set @post_half_hour = DATE_SUB(now(), INTERVAL 30 MINUTE);
    set @time_min = LPAD(FLOOR(MINUTE(@post_half_hour) / 30) * 30, 2, 0);
    set @post_half_hour = CONCAT(DATE_FORMAT(@post_half_hour, '%Y-%m-%d %H:'), @time_min, ':00');
    CALL call_customer_flow_statistic_half_hourly(@post_half_hour);
END;

ALTER EVENT CustomerFlowDB.e_customer_flow_statistic_half_hourly
	ENABLE
