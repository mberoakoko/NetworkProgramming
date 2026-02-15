cmake_minimum_required(VERSION 4.0)
project(NetworkProgramming)

set(CMAKE_CXX_STANDARD 23)

find_package(Poco REQUIRED COMPONENTS Data DataSQLite Foundation Net NetSSL Util)



add_executable(NetworkProgramming main.cpp
        src/identity.hpp
        src/socket_resource.hpp
        src/audio_pipeline.hpp
        src/reactor.hpp)

include_directories(
        Poco::Data
        Poco::DataSQLite
        Poco::Foundation
        Poco::Net
        Poco::NetSSL
        Poco::Util
)




target_link_libraries(NetworkProgramming PUBLIC
        Poco::Data
        Poco::DataSQLite
        Poco::Foundation
        Poco::Net
        Poco::NetSSL
        Poco::Util
)