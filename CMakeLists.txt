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

include(ExternalProject)
set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)

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

#-------------------------------------------------------------------------------
# Create target WaterSurfaceSimulation

#Declader new target
GMU_START_NEW_TARGET( WaterSurfaceSimulation )

#include cpp and header files
GMU_SOURCE_DIR( ${CMAKE_CURRENT_LIST_DIR}/src  )
GMU_INCLUDE_DIR( ${CMAKE_CURRENT_LIST_DIR}/include )

#include resoureces dir
GMU_SET_RESOURCE_DIR(${CMAKE_CURRENT_LIST_DIR}/resources)

#Find GPUEngine
set(GMU_TARGET_LIBRARIES geGL::geGL geUtil::geUtil SDLWindow::SDLWindow)
find_package(GPUEngine CONFIG COMPONENTS geGL geUtil SDLWindow REQUIRED)

#Build it
GMU_CONSOLE_APP_BUILD()