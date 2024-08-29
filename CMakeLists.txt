cmake_minimum_required(VERSION 3.29)
project(Gnomio)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the path to the Vcpkg toolchain file
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "C:/Dev/git/vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# Find and link libraries
find_package(OpenCV REQUIRED)
find_package(Tesseract CONFIG REQUIRED)
find_package(absl CONFIG REQUIRED)
find_package(utf8_range CONFIG REQUIRED)

# Automatically find all source and header files in the src and include directories
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h" "include/*.hpp")

# Add executable with all source files
add_executable(Gnomio ${SOURCES} ${HEADERS} Resources/resources.rc)

# Include directories for libraries
target_include_directories(Gnomio PRIVATE
        ${OpenCV_INCLUDE_DIRS}
        ${Tesseract_INCLUDE_DIRS}
        ${absl_INCLUDE_DIRS}
        ${utf8_range_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(Gnomio PRIVATE
        ${OpenCV_LIBS}
        Tesseract::libtesseract
        absl::base  # Adjust with necessary absl components
        absl::strings
        utf8_range::utf8_range
        Shcore
)

# Remove MSVC-specific flags if present
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()
