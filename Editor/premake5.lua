project "Editor"
    characterset "ASCII"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    targetdir(dirpath)
    objdir(objpath)

    files
    {
        "Source/**.c",
        "Source/**.h",
        "Source/**.hpp",
        "Source/**.cpp"
    }

    includedirs
    {
        "%{includelist.Engine}",
        "%{includelist.Editor}",
        
        "%{includelist.Vulkan}",
        "%{includelist.ImGUI}",
        "%{includelist.ImGUIzmo}",
        "%{includelist.GLI}",
        "%{includelist.GLM}",
        "%{includelist.TinyGLTF}",
        "%{includelist.EnTT}",
        "%{includelist.JSON}"
    }

    links
    {
        "Engine",
        "ImGUI",
        "ImGUIzmo"
    }

    defines 
    {
        "API_IMPORT"
    }

    filter "configurations:Debug"
        defines { "EDITOR_DEBUG" }
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        defines { "EDITOR_RELEASE" }
        runtime "Release"
        optimize "Full"

    filter "system:windows"
        optimize "Speed"
        
        defines 
        {
            "_CRT_SECURE_NO_WARNINGS",
            "PLATFORM_WINDOWS"
        }

        linkoptions { "/ignore:4006", "/ignore:4098"  }