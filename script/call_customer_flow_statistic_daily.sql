drop procedure if exists call_customer_flow_statistic_daily;
delimiter $$
create procedure call_customer_flow_statistic_daily (in local_date varchar(32))
begin
    declare counts int;
    declare total int;

    set counts = 1;
    set total = 4;

    while counts <= total do
        set @call_sql = concat('call customer_flow_statistic_daily(?, ?)');
        prepare stmt from @call_sql;
        set @par_counts = counts;
        set @par_local_date = local_date;
        execute stmt using @par_counts, @par_local_date;
        deallocate prepare stmt;

        set counts = counts + 1;
    end while;
end
$$
delimiter ;
