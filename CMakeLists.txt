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

# Add executable and source files
add_executable(Gnomio
        src/main.cpp
        Resources/resources.rc
        src/Processes/MCProcess.cpp
        src/Processes/MCProcess.h
        src/Processes/MCProcessManager.cpp
        src/Processes/MCProcessManager.h
        src/Processes/OCR/OCREngine.cpp
        src/Processes/OCR/OCREngine.h
)

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
)

# Remove MSVC-specific flags if present
if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()