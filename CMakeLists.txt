cmake_minimum_required(VERSION 3.5)
project(c11thrd)

include_directories(${CMAKE_SOURCE_DIR}/include)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wall")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    if(UNIX)
        set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -pthread")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    endif()
    if(WIN32)
        set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -DWINVER=0x0601")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWINVER=0x0601")
    endif()
endif()

set(srcfiles ${srcfiles} ${CMAKE_SOURCE_DIR}/src/threads.c)
set(srcfiles ${srcfiles} ${CMAKE_SOURCE_DIR}/src/mutex.cpp)
set(srcfiles ${srcfiles} ${CMAKE_SOURCE_DIR}/src/spinlock.c)

add_library(c11thrd ${srcfiles})

install(TARGETS c11thrd DESTINATION lib)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include)
