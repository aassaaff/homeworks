cmake_minimum_required(VERSION 3.15)
project(commands)

add_compile_options(-std=c++11)

add_library(commands src/commander.cpp src/modbus_commands.cpp include/commands/commander.h include/commands/modbus_commands.h)

find_package(Boost REQUIRED COMPONENTS system)

include_directories(include ${Boost_INCLUDE_DIRS} )

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
        infra
)
