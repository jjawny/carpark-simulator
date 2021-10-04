cmake_minimum_required(VERSION 3.20)
project(project_carpark C)

set(Warnings "−Wall −g −Wpedantic −Wextra -pedantic")
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS -lpthread ${warnings})

include_directories(.)
include_directories(src-fire-alarm-system)
include_directories(src-manager)
include_directories(src-simulator)

add_executable(MANAGER
        src-manager/display-status.c
        src-manager/display-status.h
        src-manager/man-common.h
        src-manager/manage-entrance.c
        src-manager/manage-entrance.h
        src-manager/manage-exit.c
        src-manager/manage-exit.h
        src-manager/manager.c
        src-manager/plates-hash-table.c
        src-manager/plates-hash-table.h
        config.h)

add_executable(SIMULATOR
        src-simulator/car-lifecycle.c
        src-simulator/car-lifecycle.h
        src-simulator/parking.c
        src-simulator/parking.h
        src-simulator/queue.c
        src-simulator/queue.h
        src-simulator/sim-common.h
        src-simulator/simulate-entrance.c
        src-simulator/simulate-entrance.h
        src-simulator/simulate-exit.c
        src-simulator/simulate-exit.h
        src-simulator/simulator.c
        src-simulator/sleep.c
        src-simulator/sleep.h
        src-simulator/spawn-cars.c
        src-simulator/spawn-cars.h
        config.h)

#add_executable(FIRE-ALARM-SYSTEM
        #src-fire-alarm-system/fire-alarm.c
        #test/test.c
        #config.h)

find_library(LIBRT rt)
if(LIBRT)
    target_link_libraries(MANAGER ${LIBRT})
    target_link_libraries(SIMULATOR ${LIBRT})
endif()
