project "SnowEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"

    debugdir "%{wks.location}"

    targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
    objdir "%{wks.location}/bin/%{cfg.buildcfg}/obj"

    files { "Source/**.h", "Source/**.cpp" }

    links
    {
        "SnowEngine"
    }

    includedirs
    {
        "../Engine/Source/",
        "Source/",
        "../Engine/External/imgui/",
        "../Engine/External/glm/",
        "../Engine/External/entt/src/",
        "../Engine/External/glfw/include/",
        "%{VULKAN_SDK}/Include/",
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"