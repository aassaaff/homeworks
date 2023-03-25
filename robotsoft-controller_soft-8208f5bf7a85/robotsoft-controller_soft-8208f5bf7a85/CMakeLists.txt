cmake_minimum_required(VERSION 3.15)
project(controller_soft VERSION 1.0.0 LANGUAGES CXX)

add_subdirectory(infra)
add_subdirectory(commands)
add_subdirectory(caja_logger)
add_subdirectory(caja_infra)
add_subdirectory(controller_manager)