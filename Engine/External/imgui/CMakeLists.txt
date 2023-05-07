cmake_minimum_required(VERSION 3.8)

project(imgui)

add_library(imgui STATIC
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_vulkan.cpp
)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}"
                    "${CMAKE_CURRENT_SOURCE_DIR}/backends"
                    "${CMAKE_CURRENT_SOURCE_DIR}/../glfw/include"
                    "$ENV{VULKAN_SDK}/Include/")
