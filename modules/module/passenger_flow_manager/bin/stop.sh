#!/bin/bash
acc=`pidof passenger_flow_manager`
kill -9 $acc
