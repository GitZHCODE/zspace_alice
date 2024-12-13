-- ALICE main premake file

-- Load Concrete Paths
include ("paths.lua")

-- Command line options for interopability
newoption {
    trigger = "interop",
    value = "INTEROP",
    description = "Choose interopability option",
    allowed = {
        { "Default", "Omniverse interop" },
        { "Rhino",   "Rhino and Omniverse interop" },
    },
    default = "Default"
}

-- Load premake customisations 
include (zspace_core_path.."/premake/customisation.lua")

-- Load zSpace dependency lists
include (zspace_deps_path.."/zspace_deps.lua")

-- Load Alice dependency lists
include (viewer_deps_path.."/alice_deps.lua")

--Define zSpace_Libs folder name
zSpace_Libs_Folder = "lib_zspace"

-- Set up viewer workspace
workspace "ALICE"
    architecture "x64"
    filename "zspace_alice"

    --Standard configurations
    configurations {
                    "A_Release_DLL",
                    "A_Debug_DLL",
        }
    -- Standard platforms
    platforms {
                    "None",
                    "Omniverse",
        }

    -- Platforms for Rhino InterOp
    filter {"options:interop=Rhino"}
        platforms {
                    "Rhino",
                    "Both",
        }

    filter {} -- reset filter

-- CUSTOM FUNCTION
function prependPath(p, table)
    for key, value in pairs(table) do
        table[key] = path.join(p, value)
    end
    return table
end

function GlobalCommonDefines()
    defines {"IGL_STATIC_LIBRARY", "_HAS_STD_BYTE=0", "WIN64", "ALICE", "NOMINMAX"}

    targetdir("bin/%{cfg.buildcfg}/%{cfg.platform}")
    objdir ("bin-int/%{cfg.buildcfg}/%{cfg.platform}")

    flags {"MultiProcessorCompile"}

    --DEBUG
    filter "configurations:*Debug*"
        optimize "Off"

    --RELEASE
    filter "configurations:*Release*"
        optimize "Full"
        floatingpoint "Fast"
        warnings "Off"

    --DLL
    filter {"configurations:*DLL*"}
        defines {"ZSPACE_DYNAMIC_LIBRARY", "ZSPACE_TOOLSETS_DYNAMIC_LIBRARY"}

    filter {"platforms:Rhino or Both"}
        defines {"ZSPACE_RHINO_INTEROP", "ZSPACE_LOAD_RHINO"}

    filter {"platforms:Omniverse or Both"}
        defines {"ZSPACE_USD_INTEROP"}
    filter{}
end

-- Injection functions
-- The paths in these functions are defined relative to wks.location
function get_include_dirs_injection()
    return prependPath(path.join("%{wks.location}", viewer_deps_path),  get_alice_include_dirs())
end

function get_lib_dirs_injection()
    return prependPath(path.join("%{wks.location}", viewer_deps_path), get_alice_lib_dirs())
end

function get_links_injection()
    local links = {
        "glew32.lib",
        "opengl32.lib",
        "freeglut.lib"
    }
    return links
end

include(tostring(viewer_path))

group "zSpace"
    include(tostring(zspace_core_path))
    include(tostring(zspace_toolsets_path))
