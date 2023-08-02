cmake_minimum_required(VERSION 3.1)



include_directories(
    .
    ../../CanFestival/usr
    ../../CanFestival/include/
)

set(src
    client_ftp.c
)

add_library(co_ftp ${src})

target_link_libraries(co_ftp 
    
    canopen
    
)
