cmake_minimum_required(VERSION 3.15)
project(DirtctIO_Test)

set(CMAKE_CXX_STANDARD 14)

add_executable(DirtctIO_Test main.cpp commom.h)

IF(WIN32)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${EXTRA_LIBS})
ELSEIF (UNIX AND NOT APPLE)
    TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${EXTRA_LIBS})
ENDIF()