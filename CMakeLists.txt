cmake_minimum_required(VERSION 3.20)
project(zos_vfs)

set(CMAKE_CXX_STANDARD 14)

add_executable(zos_vfs main.cpp VFSManager.cpp VFSManager.h Constants.cpp Constants.h VFSDefinitions.h StringUtils.cpp StringUtils.h)
