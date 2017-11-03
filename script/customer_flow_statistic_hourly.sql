drop procedure if exists customer_flow_statistic_hourly;
delimiter $$
create procedure customer_flow_statistic_hourly (in which_month int,
in local_time_hour varchar(32))
begin
    declare device_id_var varchar(36);
    declare data_time_hour varchar(32);
    declare sum_enter int;
    declare sum_leave int;
    declare sum_stay int;

    declare which_hour varchar(32);
    declare table_into varchar(64);

    declare done int default 0;
    declare sum_cur cursor for
    select
        device_id,
        date_hour,
        sum_e,
        sum_l,
        sum_s
    from view_hourly_temp;
    declare continue handler for not found set done = 1;

    set @view_sql = concat('create view view_hourly_temp as select device_id, date_format(data_time, \'%Y-%m-%d %H:\') date_hour, sum(enter_number) sum_e, sum(leave_number) sum_l, sum(stay_number) sum_s from t_customer_flow_original_', which_month, ' where create_date like \'', local_time_hour, '%\' group by device_id, date_hour');

    prepare view_stmt from @view_sql;
    execute view_stmt;
    deallocate prepare view_stmt;

    open sum_cur;
    fetch next from sum_cur into device_id_var, data_time_hour, sum_enter, sum_leave, sum_stay;
    while done <> 1 do
        set which_hour = concat(data_time_hour, '00:00');
        set table_into = concat('t_customer_flow_statistic_hourly_', quarter(which_hour));
        set @ins_sql = concat('insert into ', table_into, ' (id, device_id, data_time, enter_number, leave_number, stay_number) values(uuid(), ?, ?, ?, ?, ?) on duplicate key update enter_number = enter_number + ?, leave_number = leave_number + ?, stay_number = stay_number + ?');

        prepare ins_stmt from @ins_sql;
        set @par_device_id = device_id_var;
        set @par_data_time = which_hour;
        set @par_sum_enter = ifnull(sum_enter, 0);
        set @par_sum_leave = ifnull(sum_leave, 0);
        set @par_sum_stay = ifnull(sum_stay, 0);
        execute ins_stmt using @par_device_id, @par_data_time, @par_sum_enter, @par_sum_leave, @par_sum_stay, @par_sum_enter, @par_sum_leave, @par_sum_stay;
        deallocate prepare ins_stmt;

        fetch next from sum_cur into device_id_var, data_time_hour, sum_enter, sum_leave, sum_stay;
    end while;
    close sum_cur;

    drop view if exists view_hourly_temp;
end
$$
delimiter ;
