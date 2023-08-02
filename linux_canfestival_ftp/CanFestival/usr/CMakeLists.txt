cmake_minimum_required(VERSION 3.1)



include_directories(
    .
    ../include
    )

set(src
    objdict.c
    driver_can.c
    driver_timer.c
)

add_library(canopen ${src})

target_link_libraries(canopen 
    rt
    CanFestival
    pthread
)
