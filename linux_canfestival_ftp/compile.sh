#!/bin/bash


dir=`pwd`

echo "project_path:$dir"

if [ ! -d "./build" ]; then
    mkdir ./build
fi

cd ./build
rm -rf *

echo "config cross compiler envionment variables"

env_file_path="/home/user/tools/esm6800/environment-setup-cortexa7hf-neon-emtronix-linux-gnueabi"

if [ ! -e "$env_file_path" ]; then
    echo "env config file does not exist"
    exit -1
fi

source $env_file_path
echo "add env variables...:"
cat $env_file_path

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
