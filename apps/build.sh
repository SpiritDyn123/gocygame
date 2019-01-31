cur_path=`pwd`
echo ${cur_path}

build_svrs=(clustersvr gatesvr loginsvr gamesvr worldsvr dbsvr)

for data in ${build_svrs[@]}
do
    echo "buid ${data}..."
    cd ${data}
    go build -o bin/${data}
    cd ${cur_path}
done

cd ${cur_path}
echo "build all svrs success"