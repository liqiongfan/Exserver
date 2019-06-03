cmake_minimum_required(VERSION 3.11)
project(exserver C)

set(MAIN_SOURCES main.c)

include_directories(
    com/exjson
    core
    http
    events
)

aux_source_directory(core       CORE_SOURCES)
aux_source_directory(http       HTTP_SOURCES)
aux_source_directory(events     EVENT_SOURCES)
aux_source_directory(com/exjson EXJSON_SOURCES)

add_executable(
    exserver
    ${MAIN_SOURCES}
    ${CORE_SOURCES}
    ${HTTP_SOURCES}
    ${EXJSON_SOURCES}
    ${EVENT_SOURCES}
)