#===============================================================================
#
# 3D Boolean Operations: Library macros CMake file
# Author: Roman Cizmarik
#
#===============================================================================

set(OSG_LIB_NAME        "OSG")
set(OPENMESH_LIB_NAME   "OpenMesh")


#generic macro for adding include/link directories.
macro(ADD_LIB _LIBNAME)
         
    message(STATUS "Looking for library ${_LIBNAME}.")

    find_package(${_LIBNAME})
    
    if(${_LIBNAME}_FOUND)
        message("Success.")
    else()
        message(FATAL_ERROR "${_LIBNAME} was not found. Check the library folder.")
    endif() 
    
    list(APPEND BooleanOP_TARGET_INCLUDE_DIRS  ${${_LIBNAME}_INCLUDE_DIRS})
    #list(APPEND BooleanOP_TARGET_LIBRARIES ${${_LIBNAME}_LIBRARIES})
    

    if(NOT ${${_LIBNAME}_LIBRARY_DIRS} STREQUAL "")  
        
        #go through the libs, extract keyword, prepend path and link it to current target
        foreach(library ${${_LIBNAME}_LIBRARIES})
            
            if(${library} STREQUAL "debug" OR ${library} STREQUAL "optimized" OR ${library} STREQUAL "general")
                set(keyword ${library})
                continue()
            endif()
          
            list(APPEND BooleanOP_TARGET_LIBRARIES "${keyword}" "${${_LIBNAME}_LIBRARY_DIRS}/${library}")
        endforeach()
        
    endif()

endmacro()



# Find the OpenMesh library.
macro( ADD_LIB_OPENMESH)

    ADD_LIB(${OPENMESH_LIB_NAME} ${ARGV})
    
    list(APPEND BooleanOP_COMPILE_DEFINITIONS "NOMINMAX")
    list(APPEND BooleanOP_COMPILE_DEFINITIONS "_USE_MATH_DEFINES")


endmacro()


# Find OSG library
macro( ADD_LIB_OSG )

    ADD_LIB(${OSG_LIB_NAME} ${ARGV})

endmacro()

MACRO(ADD_3D_BOOLEAN_LIB )
    list(APPEND BooleanOP_TARGET_LIBRARIES ${3DBOOLEAN_LIB_TARGET_NAME})
    list(APPEND BooleanOP_TARGET_INCLUDE_DIRS ${3DBOOLEAN_LIB_INCLUDE_DIR} )
ENDMACRO( ADD_3D_BOOLEAN_LIB )

MACRO(ADD_QT_LIBS)
    list(APPEND BooleanOP_TARGET_LIBRARIES ${ARGV})
ENDMACRO(ADD_QT_LIBS)

MACRO(ADD_GLUT_LIB)
    SET(GLUT_ROOT_DIR "" CACHE STRING "Glut root dir")
    find_package(GLUT)
    if(GLUT_FOUND)
        message(${GLUT_LIBRARIES})
        message(${GLUT_INCLUDE_DIRS})
        list(APPEND BooleanOP_TARGET_LIBRARIES ${GLUT_LIBRARIES})
        list(APPEND BooleanOP_TARGET_INCLUDE_DIRS ${GLUT_INCLUDE_DIRS} )

    else()
        message("Glut not found")
    endif()
ENDMACRO(ADD_GLUT_LIB)

MACRO(ADD_OPENGL_LIB)
        list(APPEND BooleanOP_TARGET_LIBRARIES glu32.lib)
        list(APPEND BooleanOP_TARGET_LIBRARIES opengl32.lib)

ENDMACRO(ADD_OPENGL_LIB)


#===============================================================================

#Macro for initializing new libary variables
MACRO( BooleanOP_LIBRARY _LIBRARY_NAME )
    SET( BooleanOP_LIBRARY_NAME ${_LIBRARY_NAME} )
    SET( BooleanOP_LIB_PROJECT_NAME lib${_LIBRARY_NAME} )
    SET( BooleanOP_LIBRARY_SOURCES "" )
    SET( BooleanOP_LIBRARY_HEADERS "" )
    BooleanOP_START_NEW_TARGET( ${_LIBRARY_NAME} )
ENDMACRO( BooleanOP_LIBRARY )

# Macro for adding source file to library build
MACRO( BooleanOP_LIBRARY_SOURCE )
    SET( BooleanOP_LIBRARY_SOURCES ${BooleanOP_LIBRARY_SOURCES} ${ARGV} )
ENDMACRO( BooleanOP_LIBRARY_SOURCE )

# Macro for adding all header files in a specified directory to library
MACRO( BooleanOP_LIBRARY_INCLUDE_DIR _DIR )
    FILE( GLOB_RECURSE _TMP ${_DIR}/*.h ${_DIR}/*.hxx )
    list(APPEND BooleanOP_LIBRARY_HEADERS ${_TMP})
    include_directories(${BooleanOP_LIBRARY_NAME} ${_DIR} )
ENDMACRO( BooleanOP_LIBRARY_INCLUDE_DIR )

# Macro for adding all cpp files in a specified directory to library
MACRO( BooleanOP_LIBRARY_SOURCE_DIR _DIR )
    FILE( GLOB_RECURSE _TMP ${_DIR}/*.cpp ${_DIR}/*.cxx )
    list(APPEND BooleanOP_LIBRARY_SOURCES ${_TMP})
ENDMACRO( BooleanOP_LIBRARY_SOURCE_DIR )

# Final building macro
MACRO( BooleanOP_LIBRARY_BUILD )
    message(STATUS "Building library ${BooleanOP_LIBRARY_NAME} .")

    ADD_LIBRARY( ${BooleanOP_LIBRARY_NAME}
                 ${BooleanOP_LIBRARY_SOURCES}
                 ${BooleanOP_LIBRARY_HEADERS} )
    SET_TARGET_PROPERTIES( ${BooleanOP_LIBRARY_NAME} PROPERTIES
                           PROJECT_LABEL ${BooleanOP_LIB_PROJECT_NAME}
                           DEBUG_POSTFIX d)

   message(STATUS "Library ${BooleanOP_LIBRARY_NAME}: Additional include dirs: ${BooleanOP_TARGET_INCLUDE_DIRS}")
   target_include_directories(${BooleanOP_LIBRARY_NAME} PRIVATE ${BooleanOP_TARGET_INCLUDE_DIRS})

   message(STATUS "Library ${BooleanOP_LIBRARY_NAME}: Linked libraries: ${BooleanOP_TARGET_LIBRARIES}")
   target_link_libraries(${BooleanOP_LIBRARY_NAME} ${BooleanOP_TARGET_LIBRARIES} )

  message(STATUS "Library ${BooleanOP_LIBRARY_NAME}: Additional compile definitions: ${BooleanOP_COMPILE_DEFINITIONS}")
  target_compile_definitions( ${BooleanOP_LIBRARY_NAME} PRIVATE ${BooleanOP_COMPILE_DEFINITIONS} )

ENDMACRO( BooleanOP_LIBRARY_BUILD )

#===============================================================================