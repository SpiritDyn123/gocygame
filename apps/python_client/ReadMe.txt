#python简易客户端

环境：
    1.python3
    2、pip install PyQt5
    3、pip install Crypto
    4、pip install pycryptodome
    4.5、 X:\Python3安装目录\Lib\site-packages\crypto，重命名为Crypto
    5、pip install protobuf
    6、根据 pip list 显示的protobuf版本，上网搜索下载对应版本protoc

执行方法：
    1、点击python_client.bat开始运行，默认是开发环境
    2、配置为config.py，可设置 config.RUN_ENVIREMENT（开发环境，生产环境，预生产，测试环境）
    3、运行的消息会很大，默认打印到config.Log_file配置的日志文件中
	
启动出现问题及解决方案：
	1、由于可能windows protobuf的版本不一致导致启动不了，点击proto文件夹下的create.sh重新生成proto代码
