cmake_minimum_required(VERSION 3.20)
project(UltraLowLatencyTrading VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optimize for latency by default
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -flto")

find_package(Threads REQUIRED)

include_directories(include)

# Assuming we'll use a mocked ZeroMQ interface or stub if not easily fetched on windows, 
# but let's try to fetch it via FetchContent or assume it's mockable.
# For this advanced project, we will build a ZeroMQ-like interface or use a lightweight networking header
# if installing zmq is problematic. Let's start with C++ core logic first.

add_subdirectory(src)
add_subdirectory(tests)
