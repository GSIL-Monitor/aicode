drop procedure if exists call_customer_flow_statistic_half_hourly;
delimiter $$
create procedure call_customer_flow_statistic_half_hourly (in local_time_min varchar(32))
begin
    declare counts int;
    declare total int;

    set counts = 1;
    set total = 12;

    while counts <= total do
        set @call_sql = concat('call customer_flow_statistic_half_hourly(?, ?)');
        prepare stmt from @call_sql;
        set @par_counts = counts;
        set @par_local_time_min = local_time_min;
        execute stmt using @par_counts, @par_local_time_min;
        deallocate prepare stmt;

        set counts = counts + 1;
    end while;
end
$$
delimiter ;
