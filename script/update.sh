#!/bin/bash


http_update()
{
    cp update_templete.py update_http.py
    sed -i "s/_local_path/\/usr\/local\/src\/modules\/module\/http_server\/bin/g" update_http.py
    sed -i "s/_local_file/access.cgi/g" update_http.py
    sed -i "s/_remote_path/\/data\/platform\/http\/bin/g" update_http.py
    sed -i "s/_remote_run/runhttp.sh/g" update_http.py
    fab -f update_http.py start
    rm -fr update_http.py*
}

acc_update()
{
    cp update_templete.py update_acc.py
    sed -i "s/_local_path/\/usr\/local\/src\/modules\/module\/access_manager\/bin/g" update_acc.py
    sed -i "s/_local_file/access_manager/g" update_acc.py
    sed -i "s/_remote_path/\/data\/platform\/access\/bin/g" update_acc.py
    sed -i "s/_remote_run/run.sh/g" update_acc.py
    fab -f update_acc.py start
    rm -fr update_acc.py*
}

file_http_update()
{
    cp update_templete.py update_file_http.py
    sed -i "s/_local_path/\/usr\/local\/src\/modules\/module\/file_http_server\/bin/g" update_file_http.py
    sed -i "s/_local_file/filemgr.cgi/g" update_file_http.py
    sed -i "s/_remote_path/\/data\/platform\/filemgr\/bin/g" update_file_http.py
    sed -i "s/_remote_run/runfile.sh/g" update_file_http.py
    fab -f update_file_http.py start
    rm -fr update_file_http.py*
}

file_handler_update()
{
    cp update_templete.py update_file_handler.py
    sed -i "s/_local_path/\/usr\/local\/src\/modules\/module\/file_handler_server\/bin/g" update_file_handler.py
    sed -i "s/_local_file/FileHandler/g" update_file_handler.py
    sed -i "s/_remote_path/\/data\/platform\/file_handler_server\/bin/g" update_file_handler.py
    sed -i "s/_remote_run/run.sh/g" update_file_handler.py
    fab -f update_file_handler.py start
    rm -fr update_file_handler.py*
}


if [[ "http" = $1 ]]; then 
    http_update
    
elif [[ "acc" = $1 ]]; then 
    acc_update
    
elif [[ "filehttp" = $1 ]]; then 
    file_http_update
    
elif [[ "filehandler" = $1 ]]; then 
    file_handler_update
    
elif [[ "all" = $1 ]]; then 
    http_update
    acc_update
else
    echo 'Error param.'
    
fi


