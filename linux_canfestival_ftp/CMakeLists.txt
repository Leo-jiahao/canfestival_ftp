#before cmake, must "source /home/user/tools/esm6800/environment-setup-cortexa7hf-neon-emtronix-linux-gnueabi" to config env

cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_FLAGS "-std=c11")

set(src
    main.c
)

set(CMAKE_C_COMPILER
    #"/usr/local/arm/gcc-linaro-4.9.4-2017.01-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc" 
    "/home/user/tools/esm6800/sysroots/x86_64-pokysdk-linux/usr/bin/arm-emtronix-linux-gnueabi/arm-emtronix-linux-gnueabi-gcc"
)

#set exe file path
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


add_subdirectory(CanFestival/src/)
add_subdirectory(CanFestival/usr/)
add_subdirectory(co_ftp/client_ftp/)

include_directories(
    
    CanFestival/include
    CanFestival/usr
    /home/user/tools/esm6800/sysroots/x86_64-pokysdk-linux/usr/include
    ./
    co_ftp/client_ftp/
)


# set(
#     zlogpath
#     /home/user/tools/esm6800/sysroots/x86_64-pokysdk-linux/usr/lib/libzlog.so
# )

add_executable(canfestival_exe ${src})

target_link_libraries(canfestival_exe 
    co_ftp
    CanFestival
    canopen
    # ${zlogpath}
    pthread
)



