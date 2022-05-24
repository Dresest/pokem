cmake_minimum_required(VERSION 3.19)
project(proyecto_bd)

set(CMAKE_CXX_STANDARD 14)

add_executable(proyecto_bd src/RecordBasedFile/test.cc src/RecordBasedFile/PagedFileManager.cc src/RecordBasedFile/RecordBasedFileManager.cc)