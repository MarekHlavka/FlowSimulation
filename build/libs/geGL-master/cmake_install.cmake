# Install script for directory: /home/marek/School/Flow/libs/geGL-master

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
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

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/home/marek/School/Flow/build/libs/geGL-master/libgeGL.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/geGL" TYPE FILE FILES
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Fwd.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Definitions.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Buffer.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/VertexArray.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Framebuffer.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Texture.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Sampler.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/ProgramPipeline.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Shader.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Program.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/ProgramInfo.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Renderbuffer.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/OpenGL.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/OpenGLUtil.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/AsynchronousQuery.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/DebugMessage.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/OpenGLObject.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/geGL.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/OpenGLFunctionTable.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/OpenGLContext.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/OpenGLCapabilities.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/FunctionLoaderInterface.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/DefaultLoader.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/LoaderTableDecorator.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/DSATableDecorator.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/TrapTableDecorator.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/CapabilitiesTableDecorator.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/StaticCalls.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/GLSLNoise.h"
    "/home/marek/School/Flow/build/libs/geGL-master/geGL/gegl_export.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/geGL/Generated" TYPE FILE FILES
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/ContextCalls.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/ContextDeclarations.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/FunctionTableCalls.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/LoaderImplementation.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/LoaderLoadingCalls.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/NumberOfFunctions.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/MemberOpenGLFunctions.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/MemberOpenGLPFN.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/OpenGLConstants.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/OpenGLFunctions.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/OpenGLPFN.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/OpenGLTypes.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/TrapCalls.h"
    "/home/marek/School/Flow/libs/geGL-master/src/geGL/Generated/TrapImplementation.h"
    )
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/geGL/geGLTargets.cmake")
    file(DIFFERENT _cmake_export_file_changed FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/geGL/geGLTargets.cmake"
         "/home/marek/School/Flow/build/libs/geGL-master/CMakeFiles/Export/5393ca3d7db93097ba02c25a5cd379af/geGLTargets.cmake")
    if(_cmake_export_file_changed)
      file(GLOB _cmake_old_config_files "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/geGL/geGLTargets-*.cmake")
      if(_cmake_old_config_files)
        string(REPLACE ";" ", " _cmake_old_config_files_text "${_cmake_old_config_files}")
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/geGL/geGLTargets.cmake\" will be replaced.  Removing files [${_cmake_old_config_files_text}].")
        unset(_cmake_old_config_files_text)
        file(REMOVE ${_cmake_old_config_files})
      endif()
      unset(_cmake_old_config_files)
    endif()
    unset(_cmake_export_file_changed)
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/geGL" TYPE FILE FILES "/home/marek/School/Flow/build/libs/geGL-master/CMakeFiles/Export/5393ca3d7db93097ba02c25a5cd379af/geGLTargets.cmake")
  if(CMAKE_INSTALL_CONFIG_NAME MATCHES "^()$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/geGL" TYPE FILE FILES "/home/marek/School/Flow/build/libs/geGL-master/CMakeFiles/Export/5393ca3d7db93097ba02c25a5cd379af/geGLTargets-noconfig.cmake")
  endif()
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Devel" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/geGL" TYPE FILE FILES
    "/home/marek/School/Flow/build/libs/geGL-master/geGL/geGLConfig.cmake"
    "/home/marek/School/Flow/build/libs/geGL-master/geGL/geGLConfigVersion.cmake"
    )
endif()

