function get_alice_include_dirs()
    local IncludeDir = {}

    IncludeDir["ALICE_BINS"]            = "Alice_Bins/include"
    IncludeDir["FREEGLUT"]              = "freeglut/include"
    IncludeDir["GL2PS"]                 = "gl2ps/include"
    IncludeDir["GLEW"]                  = "glew/include"
    IncludeDir["LIBGIZMO"]              = "libGizmo/include"
    IncludeDir["MATRICES"]              = "matrices"
    IncludeDir["POLYCLIPPER"]           = "polyClipper"

    IncludeDir["RHINOSDK"]              = "%{rhino_dir}/inc"
    IncludeDir["MAYA"]                  = "%{maya_dir}/include"

    return IncludeDir
end

function get_alice_lib_dirs()
    local LibDir = {}

    LibDir["ALICE_BINS"]            = "Alice_Bins/bin"
    LibDir["FREEGLUT"]              = "freeglut/bin"
    LibDir["GL2PS"]                 = "gl2ps/bin"
    LibDir["GLEW"]                  = "glew/bin"
    LibDir["LIBGIZMO"]              = "libGizmo/lib"

    LibDir["RHINOSDK"]              = "%{rhino_dir}/lib/Release"
    LibDir["MAYA"]                  = "%{maya_dir}/lib"

    return LibDir
end
