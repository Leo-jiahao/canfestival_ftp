cmake_minimum_required(VERSION 3.1)



include_directories(
    .
    ../include
    ../usr
    )

set(src
    dcf.c  
    emcy.c  
    lifegrd.c  
    lss.c  
    nmtMaster.c  
    nmtSlave.c  
    objacces.c  
    pdo.c  
    sdo.c  
    states.c  
    symbols.c  
    sync.c  
    timer.c
)
# AUX_SOURCE_DIRECTORY(. src)

# add_library(CanFestival ${src}) # need include_directories

add_library(CanFestival ${src})

target_link_libraries(CanFestival 
    rt
    canopen
)