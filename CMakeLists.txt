cmake_minimum_required(VERSION 3.0.2)
project(pinecone C)

set(CMAKE_C_STANDARD 11)

include_directories(include)
file(GLOB SRC "src/*.c")
add_executable(pinecone ${SRC})
