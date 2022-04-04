cmake_minimum_required(VERSION 3.20)
project(parallel_programming_labs C)

find_package(MPI) #make it REQUIRED, if you want
add_definitions(-DOMPI_SKIP_MPICXX)

set(CMAKE_C_STANDARD 99)

add_executable(hello_world examples/hello_world.c)
target_link_libraries(hello_world PRIVATE MPI::MPI_C)

add_executable(send_receive examples/send_receive.c)
target_link_libraries(send_receive PRIVATE MPI::MPI_C)

add_executable(pi examples/pi.c)
target_link_libraries(pi PRIVATE MPI::MPI_C)

add_executable(lab1 lab1/lab1.c)
target_link_libraries(lab1 PRIVATE MPI::MPI_C)