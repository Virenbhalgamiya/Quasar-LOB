add_library(ullt_core INTERFACE)
target_include_directories(ullt_core INTERFACE ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(ullt_core INTERFACE Threads::Threads)

include(zmq_deps.cmake)

# We will add executable targets for the separate microservices
# e.g., matching_engine, market_data_feed, risk_manager, strategy_engine
add_executable(matching_engine matching_engine_main.cpp)
target_link_libraries(matching_engine PRIVATE ullt_core)

add_executable(benchmark_engine benchmark_main.cpp)
target_link_libraries(benchmark_engine PRIVATE ullt_core)
