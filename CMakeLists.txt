cmake_minimum_required(VERSION 3.20)
project(Magma LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)
find_package(X11 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(XCB REQUIRED IMPORTED_TARGET xcb)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

option(ENABLE_DEBUG "Enable custom debug mode" ON)

add_executable(Magma
        main.cpp
        ${SOURCES}
        ${HEADERS}
     )

target_include_directories(Magma PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(Magma PRIVATE
        Vulkan::Vulkan
        X11::X11
        PkgConfig::XCB
)

if(ENABLE_DEBUG)
    target_compile_definitions(Magma PRIVATE DEBUG_MODE=1)
else()
    target_compile_definitions(Magma PRIVATE DEBUG_MODE=0)
endif()