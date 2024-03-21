cmake_minimum_required(VERSION 3.28.1)
set(filename ssh)


project(filename)

set(libssh_INCLUDE_DIR /usr/local/include)
set(libssh_LIBRARY_DIR /usr/local/lib)

include_directories(${libssh_INCLUDE_DIR})
link_directories(${libssh_LIBRARY_DIR})

add_executable(main ${filename}.cpp)

target_link_libraries( main  
    
    libssh.so
)

