cur_path=`pwd`
echo ${cur_path}
all_svrs=(clustersvr gatesvr loginsvr gamesvr worldsvr dbsvr)

function start(){
    for data in ${all_svrs[@]}
    do
        echo "start ${data}..."
        cd ${data}/bin
        nohup ./${data} -log_console &
        cd ${cur_path}
    done

    cd ${cur_path}
    echo "start all svrs success"
}


function stop(){
    for data in ${all_svrs[@]}
    do
        echo "stop ${data}..."
        ps aux|grep ${data} | grep -v grep | awk '{print $2}'|xargs kill
    done

    cd ${cur_path}
    echo "stop all svrs success"
}

case "$1" in
start)
   start
   ;;
stop)
   stop
   ;;
status)
   status
   ;;
*)
   echo $"Usage: $0 {start|stop|status}"
   exit 1
esac

