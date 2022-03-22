# Install script for directory: C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Users/vishu.b/Source/Repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/install/x64-Debug")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Debug")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/build/x64-Debug/libs/eigen3/cmake_install.cmake")
  include("C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/build/x64-Debug/libs/eigen2/cmake_install.cmake")
  include("C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/build/x64-Debug/libs/tensors/cmake_install.cmake")
  include("C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/build/x64-Debug/libs/BLAS/cmake_install.cmake")
  include("C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/build/x64-Debug/libs/ublas/cmake_install.cmake")
  include("C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/build/x64-Debug/libs/gmm/cmake_install.cmake")
  include("C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/build/x64-Debug/libs/mtl4/cmake_install.cmake")
  include("C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/build/x64-Debug/libs/blitz/cmake_install.cmake")
  include("C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/build/x64-Debug/libs/tvmet/cmake_install.cmake")
  include("C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/build/x64-Debug/libs/STL/cmake_install.cmake")
  include("C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/build/x64-Debug/libs/blaze/cmake_install.cmake")
  include("C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/build/x64-Debug/data/cmake_install.cmake")

endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "C:/Users/vishu.b/source/repos/GitZHCODE/Alice2018_zSpace/ALICE_PLATFORM/src/Eigen/bench/btl/out/build/x64-Debug/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
