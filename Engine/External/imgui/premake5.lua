project "ImGui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"

    targetdir "%{wks.location}/bin/%{cfg.buildcfg}"
    objdir "%{wks.location}/bin/%{cfg.buildcfg}/obj"

    VULKAN_SDK = os.getenv("VULKAN_SDK")
    
    includedirs
    {
        ".",
        "%{VULKAN_SDK}/Include/",
        "../glfw/include/",
    }

    --[[links
    {
        "GLFW",
        "%{VULKAN_SDK}/Lib/vulkan-1.lib",
    }--]]

    files 
    { 
        "imconfig.h",
		"imgui.h",
		"imgui.cpp",
		"imgui_draw.cpp",
		"imgui_internal.h",
		"imgui_tables.cpp",
		"imgui_widgets.cpp",
		"imstb_rectpack.h",
		"imstb_textedit.h",
		"imstb_truetype.h",
		"imgui_demo.cpp",

        "backends/imgui_impl_glfw.h",
        "backends/imgui_impl_glfw.cpp",
        "backends/imgui_impl_vulkan.h",
        "backends/imgui_impl_vulkan.cpp",
    }