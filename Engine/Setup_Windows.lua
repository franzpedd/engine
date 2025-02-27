-- windows premake5 script
project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    pchheader "epch.h"
    pchsource "Source/epch.cpp"

    targetdir(dir)
    objdir(dir)
    vulkan_path = os.getenv("VULKAN_SDK");
    
    files
    {
        "Source/**.h",
        "Source/**.cpp",

        "Wrapper/**.h",
        "Wrapper/**.cpp"
    }

    includedirs
    {
        "%{includes.Engine}/Source",
        "%{includes.Engine}/Wrapper",
        "%{includes.GLM}",
        "%{includes.ImGui}",
        "%{includes.ImGuiExtra}",
        "%{includes.EnTT}",
        "%{includes.STB}",

        "%{vulkan_path}/Include",
        "%{wks.location}/Thirdparty/sdl/SDL2-2.30.2/include",
        "%{wks.location}/Thirdparty/assimp/include",
        "%{wks.location}/Thirdparty/openal/include"
    }

    links
    {
        "ImGui"
    }

    filter "configurations:Debug"
        defines { "ENGINE_DEBUG" }
        runtime "Debug"
        symbols "On"

        includedirs
        {
            "%{wks.location}/Thirdparty/assimp/build/Debug/include"
        }

        defines
        {
            "_CRT_SECURE_NO_WARNINGS"
        }

        prebuildcommands
        {
            "{COPYFILE} %{wks.location}/Thirdparty/sdl/SDL2-2.30.2/lib/x64/SDL2.dll " .. dir;
            "{COPYFILE} %{wks.location}/Thirdparty/openal/build/Debug/Debug/OpenAL32.dll " .. dir;
        }

        links
        {
            -- vulkan
            "%{vulkan_path}/Lib/vulkan-1.lib",
            "%{vulkan_path}/Lib/shaderc_shared.lib",
            -- sdl
            "%{wks.location}/Thirdparty/sdl/SDL2-2.30.2/lib/x64/SDL2.lib",
            "%{wks.location}/Thirdparty/sdl/SDL2-2.30.2/lib/x64/SDL2main.lib",
            -- assimp
            "%{wks.location}/Thirdparty/assimp/build/Debug/lib/Debug/assimp-vc143-mtd.lib",
            "%{wks.location}/Thirdparty/assimp/build/Debug/contrib/zlib/Debug/zlibstaticd.lib",
            -- open-al
            "%{wks.location}/Thirdparty/openal/build/Debug/Debug/OpenAL32.lib"
        }

        linkoptions
         {
            "/ignore:4006"
        }

    filter "configurations:Release"
        defines { "ENGINE_RELEASE" }
        runtime "Release"
        optimize "On"

        includedirs
        {
            "%{wks.location}/Thirdparty/assimp/build/Release/include"
        }

        prebuildcommands
        {
            "{COPYFILE} %{wks.location}/Thirdparty/sdl/SDL2-2.30.2/lib/x64/SDL2.dll " .. dir;
            "{COPYFILE} %{wks.location}/Thirdparty/openal/build/Release/Release/OpenAL32.dll " .. dir;
        }

        links
        {
            -- vulkan
            "%{vulkan_path}/Lib/vulkan-1.lib",
            "%{vulkan_path}/Lib/shaderc_shared.lib",
            -- sdl
            "%{wks.location}/Thirdparty/sdl/SDL2-2.30.2/lib/x64/SDL2.lib",
            "%{wks.location}/Thirdparty/sdl/SDL2-2.30.2/lib/x64/SDL2main.lib",
            -- assimp
            "%{wks.location}/Thirdparty/assimp/build/Release/lib/Release/assimp-vc143-mtd.lib",
            "%{wks.location}/Thirdparty/assimp/build/Release/contrib/zlib/Release/zlibstaticd.lib",
            -- open-al
            "%{wks.location}/Thirdparty/openal/build/Release/Release/OpenAL32.lib"
        }