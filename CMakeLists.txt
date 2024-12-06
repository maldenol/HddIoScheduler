cmake_minimum_required(VERSION 3.26)
project(krspz)

set(CMAKE_CXX_STANDARD 20)

add_executable(krspz main.cpp
        Scheduler.h
        UserProcess.h
        IORequest.h
        BufferCache.h
        Driver.h
        Scheduler.cpp
        Driver.cpp
        BufferCache.cpp
        Buffer.h
        DriverFIFO.h
        DriverFIFO.cpp
        DriverLOOK.h
        DriverLOOK.cpp
        DriverFLOOK.h
        DriverFLOOK.cpp
)
