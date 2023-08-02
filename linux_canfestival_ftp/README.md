# 1、配置说明
使用正点原子和英创的esm6800开发板完成验证。配置交叉编译环境正点原子和英创平台一致，基本通用
## 1.1、交叉工具链配置
参考esm6800开发工具安装.pdf  
shell命令  
```shell
user@ubuntu16:~/Desktop$ ./tools/esm6800-toolchain-x86_64.sh  
选择安装路径：/home/user/tools/esm6800
Proceed[Y/n]? y  
```
等待安装完成
## 1.2、配置vscode
需要安装C/C++插件等，可自行下载对应linux版本的安装文件进行离线安装  
其次我们需要在命令控制面板配置：  
C/C+++:编辑配置(UI)  
选择编译器：  
/home/user/tools/esm6800/sysroots/x86_64-pokysdk-linux/usr/bin/arm-emtronix-linux-gnueabi/arm-emtronix-linux-gnueabi-gcc  
编译器参数：  
-mfloat-abi=hard  
头文件路径：  
/home/user/tools/esm6800/sysroots/cortexa7hf-neon-emtronix-linux-gnueabi/usr/include  
/home/user/tools/esm6800/sysroots/x86_64-pokysdk-linux/usr/include  
```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "${workspaceFolder}/CanFestival/include",
                "${workspaceFolder}/CanFestival/usr",
                "/home/user/tools/esm6800/sysroots/cortexa7hf-neon-emtronix-linux-gnueabi/usr/include",
                "/home/user/tools/esm6800/sysroots/x86_64-pokysdk-linux/usr/include"
            ],
            "defines": [],
            "compilerPath": "/home/user/tools/esm6800/sysroots/x86_64-pokysdk-linux/usr/bin/arm-emtronix-linux-gnueabi/arm-emtronix-linux-gnueabi-gcc",
            "cStandard": "c11",
            "cppStandard": "c++17",
            "intelliSenseMode": "linux-gcc-x64",
            "configurationProvider": "ms-vscode.cmake-tools",
            "compilerArgs": [
                "-mfloat-abi=hard"
            ]
        }
    ],
    "version": 4
}
```
## 1.3、配置NFS

注：应该属于环境配置相关工作。仅仅提供一个思路  

1. 安装nfs服务

2. 在此路径的上级新建nfs文件
```shell
mkdir ../nfs
sudo chmod 777 ../nfs
```

3. 配置/etc/export文件
*compile.sh中的可执行文件存放路径必须保持一致*
```shell
# compile.sh
cp ../canfestival_exe ../../nfs
```
```shell
# /etc/export
/home/user/workspace/nfs *(rw,sync,no_root_squash)
```

4. 开发板和宿主机在同一局域网

5. 开发板新建mount脚本，并给执行权限
```shell
root@ATK-IMX6U:/mnt# cat ~/mount.sh 
#!/bin/bash
mount -t nfs -o nolock,vers=3 192.168.22.107:/home/user/workspace/nfs /mnt
```

# 2、使用cmake工具进行工程管理
## 2.1、安装cmake
自行在网上查找方法,一般linux发行版都会自带
## 2.2、CMakeLists.txt
原则：*只描述当前层的编译依赖规则*  
对于第三方库，如pthread,math,需要动态链接
```cmake
# 仅作为示例参考
cmake_minimum_required(VERSION 3.1)

project(uart VERSION 1.0)

#add user head files to the end of compiler's include path list
include_directories(
    ./crc
    ./data
    ./frame
    ./uart

    )

set(src
    uart.c
    )

#for all targets, link before add_library or add_executable
link_libraries(m) 

add_library(uart ${src})

#for a target
#target_link_libraries(uart m)

```
对于zlog共享库的链接，展示如下一种方法
```cmake
cmake_minimum_required(VERSION 3.1)

project(usrzlog VERSION 1.0)

include_directories(.)

include_directories(/home/user/tools/esm6800/sysroots/x86_64-pokysdk-linux/usr/include)


set(src
    usr_zlog.c
    )

set(
    zlogpath
    /home/user/tools/esm6800/sysroots/x86_64-pokysdk-linux/usr/lib/libzlog.so
)

add_library(zlog SHARED IMPORTED)
set_target_properties(zlog PROPERTIES IMPORTED_LOCATION ${zlogpath})



#for all targets, link before add_library or add_executable
#link_libraries(
#    pthread
#    zlog
#    ) 

add_library(usrzlog ${src})

target_link_libraries(usrzlog pthread zlog)

#for a target
#target_link_libraries(target pthread)

```
# 3、编译
使用shell脚本(compile.sh)进行编译，shell脚本和main.c一起处于工程根目录
## 3.1、新建build文件夹、清空build目录下之前的信息  
```shell
dir=`pwd`

echo "project_path:$dir"

if [ ! -d "./build" ]; then
    mkdir ./build
fi

cd ./build
rm -rf *

```
## 3.2、加载交叉编译工具链的环境变量
```shell
echo "config cross compiler envionment variables"

env_file_path="/home/user/tools/esm6800/environment-setup-cortexa7hf-neon-emtronix-linux-gnueabi"

if [ ! -e "$env_file_path" ]; then
    echo "env config file does not exist"
    exit -1
fi

source $env_file_path
echo "add env variables...:"
cat $env_file_path

```
## 3.3、检查路径、编译、退出、拷贝到nfs路径
```shell
main_file="$dir/main.c"

if [ ! -e $main_file ]; then
    echo "compile.sh file run in a wrong path"
    exit -1
fi
echo "find main file:$main_file"


cmake ../

make

cp ../canfestival_exe ../../nfs

exit 0
```
## 3.4、配置vscode的tasks.json用于编译
配置完成后可以快速执行
```json
{
    "tasks": [
        {
            "type": "cppbuild",
            "label": "compile by shell",
            "command": "bash",
            "args": [
                "${workspaceRoot}/compile.sh"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "options": {
                "cwd": "${fileDirname}"
            },
            "detail": "shell script compile project"
        }
    ],
    "version": "2.0.0"
}
```
## 3.5、总结
编译：  
```shell
user@ubuntu16:~/workspace/canfestival_demo$ ./compile.sh 
project_path:/home/user/workspace/canfestival_demo
config cross compiler envionment variables
add env variables...:
...
find main file:/home/user/workspace/canfestival_demo/main.c
...
[100%] Built target canfestival_exe
```
编译成功生成的可执行文件位于工程的根目录下，可查看
```shell
user@ubuntu16:~/workspace/canfestival_demo$ ls -l
total 172
drwxrwxr-x 4 user user   4096 Jun  1 15:08 build
drwxrwxr-x 5 user user   4096 May 30 14:32 CanFestival
-rwxrwxr-x 1 user user 130028 Jun  1 15:08 canfestival_exe
-rw-rw-r-- 1 user user   1022 May 31 17:13 CMakeLists.txt
-rwxrwxr-x 1 user user    644 Jun  1 11:30 compile.sh
-rw-rw-r-- 1 user user  11908 Jun  1 14:51 main.c
drwxrwxr-x 2 user user   4096 May 31 16:51 output
-rw-rw-r-- 1 user user   9501 Jun  1 16:47 README.md
```

# 4、工程说明
## 4.1、文件管理
```shell
user@ubuntu16:~/workspace/canfestival_demo$ tree -L 3 -I "build"
.
├── CanFestival
│   ├── include
│   │   ├── can_driver.h
│   │   ├── can.h
│   │   ├── data.h
│   │   ├── dcf.h
│   │   ├── def.h
│   │   ├── emcy.h
│   │   ├── lifegrd.h
│   │   ├── lss.h
│   │   ├── nmtMaster.h
│   │   ├── nmtSlave.h
│   │   ├── objacces.h
│   │   ├── objdictdef.h
│   │   ├── pdo.h
│   │   ├── sdo.h
│   │   ├── states.h
│   │   ├── sync.h
│   │   ├── sysdep.h
│   │   ├── timer.h
│   │   └── timers_driver.h
│   ├── src
│   │   ├── CMakeLists.txt
│   │   ├── dcf.c
│   │   ├── emcy.c
│   │   ├── lifegrd.c
│   │   ├── lss.c
│   │   ├── nmtMaster.c
│   │   ├── nmtSlave.c
│   │   ├── objacces.c
│   │   ├── objaccessinternal.h
│   │   ├── pdo.c
│   │   ├── sdo.c
│   │   ├── states.c
│   │   ├── symbols.c
│   │   ├── sync.c
│   │   └── timer.c
│   └── usr
│       ├── applicfg.h
│       ├── canfestival.h
│       ├── CMakeLists.txt
│       ├── config.h
│       ├── driver_can.c
│       ├── driver_timer.c
│       ├── objdict.c
│       ├── objdict.h
│       └── timerscfg.h
├── canfestival_exe
├── CMakeLists.txt
├── co_ftp
│   ├── client_ftp
│   │   ├── client_ftp.c
│   │   ├── client_ftp.h
│   │   ├── CMakeLists.txt
│   │   └── test_demo.txt
│   ├── README.md
│   └── server_ftp
│       ├── server_ftp.c
│       ├── server_ftp.h
│       └── test_demo.txt
├── compile.sh
├── main.c
├── output
├── README.md
└── test.bin

8 directories, 57 files
```
## 4.2、源码bug
canfestival test demo  
在PDO.c 的223行发现一些问题，修正后如下
```c
                // while (numMap < READ_UNS8(d->objdict, offsetObjdict, 0))
                while (numMap < READ_UNS8(d->objdict, offsetObjdict + numPdo, 0))
```

# 5、一些注意事项
如果遇到 can 发送 No buffer space available，这个问题是系统给的缓冲队列空间不足
```shell
cd /sys/class/net/can0
cat tx_queue_len
10
echo 4096 >tx_queue_len
```
