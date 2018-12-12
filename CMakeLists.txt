cmake_minimum_required(VERSION 2.6)
project(OpenGL-projects)

set(CMAKE_CXX_STANDARD 11)
set(CPACK_GENERATOR "TGZ")

if(WIN32)
   message(STATUS "Compilation for a Windows system.")
   set(OPENGL_ROOT_DIR "C:/Program Files/Common Files/MinGW/freeglut")
   include_directories("${OPENGL_ROOT_DIR}/include")
   link_directories("${OPENGL_ROOT_DIR}/lib")
elseif(UNIX)
   message(STATUS "Compilation for an UNIX like system.")
   find_package(OpenGL REQUIRED)
   include_directories(${OPENGL_INCLUDE_DIR})

   find_package(GLUT REQUIRED)
   include_directories(${GLUT_INCLUDE_DIR})
endif()


find_package(PostgreSQL REQUIRED)
include_directories(${PostgreSQL_INCLUDE_DIR})


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


add_compile_options("-O3")

# Uncomment next line to compile without using C++11
#add_definitions(-D__Cpp11__)


add_subdirectory(src)
include_directories(src /opt/local/include)

include(CPack)
