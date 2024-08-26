cmake_minimum_required(VERSION 3.29)
project(Gnomio)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_TOOLCHAIN_FILE "C:/Users/amdro/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Manually set CURL paths
set(CURL_INCLUDE_DIR "C:/Users/amdro/vcpkg/installed/x64-windows/include")
set(CURL_LIBRARY "C:/Users/amdro/vcpkg/installed/x64-windows/lib/libcurl.lib")

add_executable(Gnomio src/main.cpp
        Resources/resources.rc
        src/Processes/MCProcess.cpp
        src/Processes/MCProcess.h
)
target_link_libraries(Gnomio PRIVATE ${CURL_LIBRARY})