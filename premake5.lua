workspace "SnowEngine"
    architecture "x64"
    configurations { "Debug", "Release" }

include "Engine"
include "Editor"

group "External"
    include "Engine/External/glfw"
    include "Engine/External/imgui"