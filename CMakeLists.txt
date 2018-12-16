cmake_minimum_required(VERSION 3.10)
project(OpenGL-projects)

set(CMAKE_CXX_STANDARD 11)
set(CPACK_GENERATOR "TGZ")

if(WIN32)
   message(STATUS "Compilation for a Windows system.")
   set(OPENGL_ROOT_DIR "C:/Program Files/Common Files/MinGW/freeglut")
   include_directories("${OPENGL_ROOT_DIR}/include")
   link_directories("${OPENGL_ROOT_DIR}/lib")

elseif(APPLE)
   message(STATUS "Compilation for a MAC using port utilities.")
  #find_package(PostgreSQL REQUIRED)
   set(PostgreSQL_INCLUDE_DIR "/opt/local/include/postgresql10")
   file(GLOB_RECURSE PostgreSQL_LIBRARIES "/opt/local/lib/postgresql10/*.dylib")
   include_directories(${PostgreSQL_INCLUDE_DIR})

  #find_package(OpenGL REQUIRED)
   set(OPENGL_INCLUDE_DIR "/opt/local/include")
   set(OPENGL_LIBRARIES   "/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
   set(GLUT_LIBRARIES     "/opt/local/lib/libglut.dylib")
   set(GLEW_LIBRARIES     "/opt/local/lib/libGLEW.dylib")
   set(GLFW_LIBRARIES     "/opt/local/lib/libglfw.dylib")
   message(STATUS "Found OPENGL_INCLUDE_DIR=${OPENGL_INCLUDE_DIR}")
   include_directories(${OPENGL_INCLUDE_DIR})

elseif(UNIX)
   message(STATUS "Compilation for an UNIX like system.")

   find_package(PostgreSQL REQUIRED)
   include_directories(${PostgreSQL_INCLUDE_DIR})

   find_package(OpenGL REQUIRED)
   include_directories(${OPENGL_INCLUDE_DIR})

   find_package(GLUT REQUIRED)
   include_directories(${GLUT_INCLUDE_DIR})

   find_package(GLEW REQUIRED)
   include_directories(${GLEW_INCLUDE_DIR})

   #find_package(GLFW REQUIRED)
   set(GLFW_INCLUDE_DIR "/usr/include")
   set(GLFW_LIBRARIES "/usr/lib/x86_64-linux-gnu/libglfw.so")
   include_directories(${GLFW_INCLUDE_DIR})

endif()


if(__OPENNN_MPI__)
   find_package(MPI)
   if(MPI_FOUND)
        message("Using MPI")
        set(CMAKE_CXX_COMPILER mpicxx)
        set(CMAKE_C_COMPILER mpicc)
        set(CMAKE_CXX_COMPILE_FLAGS "${CMAKE_CXX_COMPILE_FLAGS} ${MPI_COMPILE_FLAGS}")
        set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} ${MPI_LINK_FLAGS}")
        add_definitions(-D__OPENNN_MPI__ )
    endif()
endif()

if(__OPENNN_OMP__)
    message("Using OpenMP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")#${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")#${OpenMP_CXX_FLAGS}")
    set(CMAEK_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()


add_compile_options("-O0")
add_compile_options("-g2")

# Uncomment next line to compile without using C++11
#add_definitions(-D__Cpp11__)


add_subdirectory(src)
include_directories(src /opt/local/include)

include(CPack)
