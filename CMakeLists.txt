cmake_minimum_required(VERSION 3.16)
project(PEP_HPC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")


include_directories(src concurrentqueue)


add_executable (demo_tasks demos/demo_tasks.cc 
                src/taskmanager.cc src/timer.cc)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(demo_tasks PRIVATE Threads::Threads)
                
target_sources (demo_tasks PUBLIC src/taskmanager.h src/timer.h)




add_executable (demo_simd demos/demo_simd.cc
                src/timer.cc)
                
target_sources (demo_simd PUBLIC src/simd.h src/timer.h)


add_executable (simd_timings demos/simd_timings.cc)
target_sources (simd_timings PUBLIC src/simd.h)


add_executable (timing_mem demos/timing_mem.cc)
target_sources (timing_mem PUBLIC src/simd.h)

