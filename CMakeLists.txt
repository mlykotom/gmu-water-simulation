#===============================================================================
#
# Water Surface Simulation CMake file
# Authors: Roman Cizmarik, Tomas Mlynaric
#
#===============================================================================

#-------------------------------------------------------------------------------
# Header and basic settings

cmake_minimum_required ( VERSION 3.7 )

message(STATUS "Using CMake version ${CMAKE_VERSION}")

set(CMAKE_CONFIGURATION_TYPES Release Debug RelWithDebInfo CACHE TYPE INTERNAL )

# Main project name
project( WaterSurfaceSimulation )


message(STATUS "  C: '${CMAKE_C_COMPILER}'")
message(STATUS "  C++: '${CMAKE_CXX_COMPILER}'")

# Add paths to custom cmake scripts
set( GMU_CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules" )
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${GMU_CMAKE_MODULE_PATH})

# Include needed macros
include( ${GMU_CMAKE_MODULE_PATH}/ApplicationMacros.cmake )

#enabling C++ 11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)       #on mac this adds flag -std=c++11 instead of -std=gnu++11

# adds current source/build directories to module's includes
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

#-------------------------------------------------------------------------------
# Create target WaterSurfaceSimulation

#Declader new target
GMU_START_NEW_TARGET( WaterSurfaceSimulation )

#include cpp and header files
GMU_SOURCE_DIR( ${CMAKE_CURRENT_LIST_DIR}/src  )
GMU_INCLUDE_DIR( ${CMAKE_CURRENT_LIST_DIR}/include )
GMU_QT_RES_DIR(${CMAKE_CURRENT_LIST_DIR}/resources )

#include resoureces dir
GMU_SET_RESOURCE_DIR(${CMAKE_CURRENT_LIST_DIR}/resources)

#Qt
# Next lines needed for building all Qt projects
find_package( Qt5Core REQUIRED )
find_package( Qt5Gui REQUIRED )
find_package( Qt5Widgets REQUIRED )
find_package( Qt5OpenGL REQUIRED )

find_package( Qt53DCore REQUIRED )
find_package( Qt53DExtras REQUIRED )
find_package( Qt53DRender REQUIRED )
find_package( Qt53DInput REQUIRED )

find_package(OpenCL REQUIRED)

include_directories( ${OpenCL_INCLUDE_DIRS} )

ADD_LIBS(
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::OpenGL
	Qt5::3DCore
	Qt5::3DExtras
	Qt5::3DRender
	Qt5::3DInput
	OpenCL::OpenCL
)

if( __APPLE__ )
    find_package(OpenMP)
    if (OPENMP_FOUND)
	    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS} -Wno-ignored-attributes")
	    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS} -Wno-ignored-attributes")
    endif()
endif()

if( WIN32 )
	ADD_LIBS(Qt5::WinMain)
endif()

add_definitions( ${QT_DEFINITIONS} )
include_directories( ${CMAKE_BINARY_DIR} )


# using Qt meta-system (precompiler)
#QT5_ADD_RESOURCES( RESOURCES ${PT_RES} )
GMU_UI_DIR( ${CMAKE_CURRENT_LIST_DIR}/ui )
QT5_WRAP_UI( UIS_HDRS ${GMU_UI} )

#include_directories( ${CMAKE_CURRENT_LIST_DIR}/include/CL )

#Build it
GMU_WINDOWS_APP_BUILD()