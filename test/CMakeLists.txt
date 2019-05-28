cmake_minimum_required(VERSION 3.11)
project(sockets C)

set(CMAKE_C_STANDARD 99)

set(MAIN_SOURCE socket_client.c)

aux_source_directory(../kernel KERNEL_DIRS)
aux_source_directory(../kernel/net KERNEL_NET_DIRS)

add_executable(client ${MAIN_SOURCE} ${KERNEL_DIRS} ${KERNEL_NET_DIRS})