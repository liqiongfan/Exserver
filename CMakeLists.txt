cmake_minimum_required(VERSION 3.13)
project(sockets C)

set(CMAKE_C_STANDARD 99)

aux_source_directory(kernel KERNEL_DIRS)
aux_source_directory(kernel/net KERNEL_NET_DIRS)

add_executable(sockets main.c ${KERNEL_DIRS} ${KERNEL_NET_DIRS})