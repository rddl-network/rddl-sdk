cmake_minimum_required(VERSION 3.11.4)
project(rddlSDK)

# Check for Linux platform
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    add_compile_definitions(LINUX_MACHINE)
endif()

set(TARGET_GROUP main CACHE STRING "Group to build")

add_compile_options(-DuECC_VLI_NATIVE_LITTLE_ENDIAN=0 -Wall -Wextra  -O0 -g --coverage -fsanitize=address -fno-omit-frame-pointer)
SET(CMAKE_EXE_LINKER_FLAGS "-DuECC_VLI_NATIVE_LITTLE_ENDIAN=0 -Wall -Wextra  -O0 -g --coverage -fsanitize=address -fno-omit-frame-pointer ${CMAKE_EXE_LINKER_FLAGS}")
set(CMAKE_BUILD_TYPE Debug)

# Include librddl as a subdirectory
add_subdirectory(libs/librddl)

# Create a library from your source files (excluding test/main.c)
add_library(rddlSDK STATIC src/platform/linux/rddlSDKAbst.c src/rddlSDKUtils.c src/rddlSDKAPI.c)

# Link with the librddl library
target_link_libraries(rddlSDK RDDLC protobuf-c)

# Specify include directories for librddl and its submodules
target_include_directories(rddlSDK PUBLIC
    include/
    libs/librddl/inc
    libs/librddl/libs/trezor-crypto/
    libs/librddl/libs/trezor-crypto/ed25519-donna/
    libs/librddl/
    libs/librddl/libs/json-maker
    libs/librddl/libs/tiny-json
    libs/librddl/libs/base64
    libs/librddl/libs/proto
    /usr/include
)

# Include the CMakeLists.txt from the test directory
add_subdirectory(test)
