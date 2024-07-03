--#############__ZSPACE_ALICE__#############
project "ALICE"
    location "project"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    --zSpace Libs
    dependson ("zSpace_Core")
    dependson ("zSpace_Interface")
    dependson ("zSpace_InterOp")
    dependson ("zSpace_Toolsets")

    GlobalCommonDefines()

    path_from_alice_to_workspace = path.getrelative(path.join("%{wks.location}", viewer_path), "%{wks.location}") -- "../" 
    path_from_alice_to_exe = path.join(path_from_alice_to_workspace, exe_path) -- "../EXE/"
    path_from_alice_to_alice_deps = path.join(path_from_alice_to_workspace, viewer_deps_path) -- "../Dependencies/"
    path_from_alice_to_zspace_deps = path.join(path_from_alice_to_workspace, zspace_deps_path) -- "../Dependencies/ZSPACE_DEPENDENCIES/"
    path_from_alice_to_zspace_core = path.join(path_from_alice_to_workspace, zspace_core_path) -- "../Dependencies/ZSPACE_CORE/"
    path_from_alice_to_zspace_toolsets = path.join(path_from_alice_to_workspace, zspace_toolsets_path) -- "../Dependencies/ZSPACE_TOOLSETS/"

    --Reset TargetDir
    targetdir("%{path_from_alice_to_exe}")
    targetname ("%{prj.name}")

    debugdir "%{path_from_alice_to_exe}"

    -- Redefine to keep paths relative.
    AliceIncludeDir = prependPath(path_from_alice_to_alice_deps, get_alice_include_dirs())
    AliceLibDir = prependPath(path_from_alice_to_alice_deps, get_alice_lib_dirs())

    files
    {
        -- Alice files
        "src/**.h",
        "src/**.cpp",

        -- Sketch files (change to relative?)
        "../userSrc/**.h",
        "../userSrc/**.cpp",
    }

    removefiles {"**/main_test.cpp"}

    --###__BASE__###
    includedirs
    {
        "%{AliceIncludeDir.ALICE_BINS}",
        "%{AliceIncludeDir.FREEGLUT}",
        "%{AliceIncludeDir.GL2PS}",
        "%{AliceIncludeDir.GLEW}",
        "%{AliceIncludeDir.LIBGIZMO}",
        "%{AliceIncludeDir.MATRICES}",
        "%{AliceIncludeDir.POLYCLIPPER}",

        "%{AliceIncludeDir.MAYA}",

        "%{path_from_alice_to_zspace_core}/src/headers",
        "%{path_from_alice_to_zspace_toolsets}/src/headers",

        "src/",
        "src/alice",
        "%{path_from_alice_to_alice_deps}"
    }

    -- Add zSpace includes
    includedirs {prependPath(path_from_alice_to_zspace_deps, get_zspace_include_dirs())}

    libdirs
    {
        "%{AliceLibDir.ALICE_BINS}",
        "%{AliceLibDir.FREEGLUT}",
        "%{AliceLibDir.GL2PS}",
        "%{AliceLibDir.GLEW}",
        "%{AliceLibDir.LIBGIZMO}",

        "%{AliceLibDir.MAYA}",

        "%{path_from_alice_to_zspace_core}/bin/%{cfg.buildcfg}",
        "%{path_from_alice_to_zspace_toolsets}/bin/%{cfg.buildcfg}",
        "%{path_from_alice_to_exe}",

        "src/"
    }

    -- zSpace build libdirs
    libdirs
    {
        "%{path_from_alice_to_zspace_core}/bin/%{cfg.buildcfg}/%{cfg.platform}",
        "%{path_from_alice_to_zspace_toolsets}/bin/%{cfg.buildcfg}/%{cfg.platform}",
    }

    --Include zSpace_Core lib paths
    libdirs {prependPath(path_from_alice_to_zspace_deps, get_zspace_lib_dirs())}

    links
    {
        "sqlite3.lib",
        "glew32.lib",
        "ALICE_DLL.lib",
        "ALICE_RobotDll.lib",
        "opengl32.lib",
        "gl2ps.lib",
        "libgizmo.lib",
        "igl.lib",
        "freeglut.lib",
        -- zSpace
        "zSpace_Core.lib",
        "zSpace_Interface.lib",
        "zSpace_Toolsets.lib",
        "zSpace_InterOp.lib",
    }
    --##############

    --###__Omniverse__###
    filter {"platforms:Omniverse or Both"}
        -- Add omniverse includes
        includedirs {prependPath(path_from_alice_to_zspace_deps, get_omniverse_includes())}

        -- Add omniverse libdirs
        libdirs {prependPath(path_from_alice_to_zspace_deps, get_omniverse_libdirs())}

        -- Add omniverse links
        links {get_omniverse_links()}
    --###################

    --###__RHINO__###
    filter {"platforms:Rhino or Both"}
        includedirs
        {
            "%{AliceIncludeDir.RHINOSDK}",
        }

        libdirs
        {
            "%{AliceLibDir.RHINOSDK}",
        }

        delayloaddlls
        {
            "opennurbs.dll",
            "RhinoCore.dll",
            "RhinoLibrary.dll"
        }
        links
        {
            "opennurbs.lib",
            "RhinoCore.lib",
            "RhinoLibrary.lib",
        }
    filter {}

    debugenvs
    {
        "PATH=%{wks.location}%{exe_path}\\lib_zspace;%{wks.location}%{exe_path}\\lib_omniverse;%PATH%"
    }

    postbuildcommands
    {
        -- Copy ALICE_DLLS dlls, freeglut, gl2ps
        "{COPYDIR} %{wks.location}%{zspace_deps_path}\\SQLITE\\dll %{wks.location}%{exe_path}\\lib_zspace",
        "{COPYFILE} %{wks.location}%{viewer_deps_path}\\Alice_Bins\\bin\\ALICE_DLL.dll %{wks.location}%{exe_path}\\lib_zspace\\ALICE_DLL.dll",
        "{COPYFILE} %{wks.location}%{viewer_deps_path}\\Alice_Bins\\bin\\ALICE_RobotDll.dll %{wks.location}%{exe_path}\\lib_zspace\\ALICE_RobotDll.dll",
        "{COPYFILE} %{wks.location}%{viewer_deps_path}\\freeglut\\bin\\freeglut.dll %{wks.location}%{exe_path}\\lib_zspace\\freeglut.dll",
        "{COPYFILE} %{wks.location}%{viewer_deps_path}\\gl2ps\\bin\\gl2ps.dll %{wks.location}%{exe_path}\\lib_zspace\\gl2ps.dll",
        "{COPYFILE} %{wks.location}%{viewer_deps_path}\\glew\\bin\\glew32.dll %{wks.location}%{exe_path}\\lib_zspace\\glew32.dll",
    }

    --zSpace Post Build commands
    postbuildcommands
    {
        "{COPYFILE} %{wks.location}%{zspace_core_path}\\bin\\%{cfg.buildcfg}\\%{cfg.platform}\\zSpace_Core.dll %{wks.location}%{exe_path}\\lib_zspace\\zSpace_Core.dll",
        "{COPYFILE} %{wks.location}%{zspace_core_path}\\bin\\%{cfg.buildcfg}\\%{cfg.platform}\\zSpace_Interface.dll %{wks.location}%{exe_path}\\lib_zspace\\zSpace_Interface.dll",
        "{COPYFILE} %{wks.location}%{zspace_core_path}\\bin\\%{cfg.buildcfg}\\%{cfg.platform}\\zSpace_InterOp.dll %{wks.location}%{exe_path}\\lib_zspace\\zSpace_InterOp.dll",
        "{COPYFILE} %{wks.location}%{zspace_toolsets_path}\\bin\\%{cfg.buildcfg}\\%{cfg.platform}\\zSpace_Toolsets.dll %{wks.location}%{exe_path}\\lib_zspace\\zSpace_Toolsets.dll",
    }
