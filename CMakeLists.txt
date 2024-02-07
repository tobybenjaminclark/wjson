cmake_minimum_required(VERSION 3.26)
project(wjson C)

set(CMAKE_C_STANDARD 90)

add_executable(wjson main.c
        wjson.c
        wjson.h)
