CREATE
	DEFINER = 'root'@'%'
EVENT CustomerFlowDB.e_customer_flow_statistic_daily
	ON SCHEDULE EVERY '1' DAY
	STARTS '2017-08-26 00:15:00'
	COMMENT '���ɿ�����Ϣ��ͳ�Ʊ�'
	DO
BEGIN
    set @post_day = DATE_SUB(current_date(), INTERVAL 1 day);
    CALL call_customer_flow_statistic_daily(@post_day);
END;

ALTER EVENT CustomerFlowDB.e_customer_flow_statistic_daily
	ENABLE
