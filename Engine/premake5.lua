project "SnowEngine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    
    targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
    objdir "%{wks.location}/bin/%{cfg.buildcfg}/obj"

    files { "Source/**.h", "Source/**.cpp" }

    VULKAN_SDK = os.getenv("VULKAN_SDK")

    includedirs 
    {
        "Source/",
        "External/stb/",
        "External/imgui/",
        "External/glm/",
        "External/glfw/include/",
        "External/entt/src/",
        "%{VULKAN_SDK}/Include/",
    }

    links
    {
        "GLFW",
        "ImGui",
        "%{VULKAN_SDK}/Lib/vulkan-1.lib",
    }

    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
        links 
        {
            "%{VULKAN_SDK}/Lib/shaderc_sharedd.lib",
            "%{VULKAN_SDK}/Lib/spirv-cross-cored.lib",
            "%{VULKAN_SDK}/Lib/spirv-cross-glsld.lib",
        }

    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
        links 
        {
            "%{VULKAN_SDK}/Lib/shaderc_shared.lib",
            "%{VULKAN_SDK}/Lib/spirv-cross-core.lib",
            "%{VULKAN_SDK}/Lib/spirv-cross-glsl.lib",
        }

