cmake_minimum_required(VERSION 3.15)
project(caja_logger)

add_compile_options(-std=c++11)

# find_package(Boost REQUIRED COMPONENTS system)

find_package(spdlog REQUIRED)

# include_directories(include ${Boost_INCLUDE_DIRS})

set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} src/caja_logger/caja_logger.cpp)

target_include_directories( ${PROJECT_NAME}
    PUBLIC ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(caja_logger PRIVATE spdlog::spdlog_header_only)