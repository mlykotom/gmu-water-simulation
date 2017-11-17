#===============================================================================
#
# Water Surface Simulation CMake file
# Authors: Roman Cizmarik, Tomas Mlynaric
#
#===============================================================================

# Initializes variables for new target
MACRO( GMU_START_NEW_TARGET _TARGET_NAME )
    SET( GMU_TARGET_NAME ${_TARGET_NAME} )
    SET( GMU_TARGET_LIBRARIES "" )
    SET( GMU_TARGET_INCLUDE_DIRS "" )
    SET( GMU_COMPILE_DEFINITIONS "")
ENDMACRO( GMU_START_NEW_TARGET )

# Initializes variables for new executable
MACRO(GMU_Executable _TARGET_NAME)
    SET( GMU_SOURCES "" )
    SET( GMU_HEADERS "" )
    SET( GMU_UI "" )
    SET( GMU_RES "" )
    SET( GMU_LIBS "" )
    SET( GMU_RESOURCES "")
    GMU_START_NEW_TARGET( ${_TARGET_NAME} )
ENDMACRO( GMU_Executable )

# Macro for adding all cpp files in a specified directory to project
MACRO( GMU_SOURCE_DIR _DIR )
    FILE( GLOB_RECURSE _TMP ${_DIR}/*.cpp ${_DIR}/*.cxx )
    list(APPEND GMU_SOURCES ${_TMP})
ENDMACRO( GMU_SOURCE_DIR )

# Macro for adding all header files in a specified directory to project
MACRO( GMU_INCLUDE_DIR _DIR )
    FILE( GLOB_RECURSE _TMP ${_DIR}/*.h ${_DIR}/*.hxx )
    list(APPEND GMU_HEADERS ${_TMP})
    include_directories(${PROJECT_NAME} ${_DIR} )
ENDMACRO( GMU_INCLUDE_DIR )

# Macro for adding additional directory to project
MACRO( GMU_ADD_INCLUDE_DIR _DIR )
    list(APPEND GMU_TARGET_INCLUDE_DIRS ${_DIR})
ENDMACRO( GMU_ADD_INCLUDE_DIR )

# Macro for setting resource directory to project
MACRO( GMU_SET_RESOURCE_DIR _DIR )
    SET(GMU_RESOURCES ${_DIR})
ENDMACRO( GMU_SET_RESOURCE_DIR )

# Macro for adding all ui files in a specified directory to project
MACRO( GMU_UI_DIR _DIR )
    FILE( GLOB_RECURSE _TMP ${_DIR}/*.ui)
    list(APPEND GMU_UI ${_TMP})
ENDMACRO( GMU_UI_DIR )

# Macro for adding all resource files in a specified directory to project
MACRO( GMU_RES_DIR _DIR )
    FILE( GLOB_RECURSE _TMP ${_DIR}/*.qrc)
    list(APPEND GMU_RES ${_TMP})
ENDMACRO( GMU_RES_DIR )

# Macro for adding library to project
MACRO( GMU_ADD_LIB _LIB )
    list(APPEND GMU_LIBS ${_LIB})
ENDMACRO( GMU_ADD_LIB )

  # Building console app macro
MACRO( GMU_CONSOLE_APP_BUILD)
    message(STATUS "Building executable ${GMU_TARGET_NAME}")

    ADD_EXECUTABLE( ${GMU_TARGET_NAME}
                    ${GMU_SOURCES}
                    ${GMU_HEADERS} 
                    )

    SET_TARGET_PROPERTIES( ${GMU_TARGET_NAME}
                           PROPERTIES LINKER_LANGUAGE CXX
                         #  DEBUG_POSTFIX d
                           )

   message(STATUS "Executable ${GMU_TARGET_NAME}: Additional include dirs: ${GMU_TARGET_INCLUDE_DIRS}, ${GMU_RESOURCES}")
   target_include_directories(${GMU_TARGET_NAME} PRIVATE ${GMU_TARGET_INCLUDE_DIRS})

   #include resources dirs
   target_include_directories(${GMU_TARGET_NAME} PRIVATE ${GMU_RESOURCES} )
   set_target_properties(${GMU_TARGET_NAME} PROPERTIES COMPILE_DEFINITIONS "APP_RESOURCES=\"${GMU_RESOURCES}\"")

    message(STATUS "Executable ${GMU_TARGET_NAME}: Linked libraries: ${GMU_TARGET_LIBRARIES}")
    target_link_libraries(${GMU_TARGET_NAME} ${GMU_TARGET_LIBRARIES} )

    message(STATUS "Executable ${GMU_TARGET_NAME}: Additional compile definitions: ${GMU_COMPILE_DEFINITIONS}")
    target_compile_definitions( ${GMU_TARGET_NAME} PRIVATE ${GMU_COMPILE_DEFINITIONS} )

#console application
    set_target_properties(${GMU_TARGET_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    set_target_properties(${GMU_TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
    set_target_properties(${GMU_TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")
    set_target_properties(${GMU_TARGET_NAME} PROPERTIES RELWITHDEBINFO "/SUBSYSTEM:CONSOLE")
    set_target_properties(${GMU_TARGET_NAME} PROPERTIES MINSIZEREL "/SUBSYSTEM:CONSOLE")

ENDMACRO( GMU_CONSOLE_APP_BUILD )

  # Building windows app macro
MACRO( GMU_WINDOWS_APP_BUILD)
    message(STATUS "Building executable ${GMU_TARGET_NAME}")

    ADD_EXECUTABLE( ${GMU_TARGET_NAME}
                    ${GMU_SOURCES}
                    ${GMU_HEADERS} 
                    ${UIS_HDRS}        
                    )

    SET_TARGET_PROPERTIES( ${GMU_TARGET_NAME}
                           PROPERTIES LINKER_LANGUAGE CXX
                         #  DEBUG_POSTFIX d
                           )

   message(STATUS "Executable ${GMU_TARGET_NAME}: Additional include dirs: ${GMU_TARGET_INCLUDE_DIRS}, ${GMU_RESOURCES}")
   target_include_directories(${GMU_TARGET_NAME} PRIVATE ${GMU_TARGET_INCLUDE_DIRS})
   target_include_directories(${GMU_TARGET_NAME} PRIVATE ${GMU_RESOURCES} )


    message(STATUS "Executable ${GMU_TARGET_NAME}: Linked libraries: ${GMU_TARGET_LIBRARIES}")
    target_link_libraries(${GMU_TARGET_NAME} ${GMU_TARGET_LIBRARIES} )

    message(STATUS "Executable ${GMU_TARGET_NAME}: Additional compile definitions: ${GMU_COMPILE_DEFINITIONS}")
    target_compile_definitions( ${GMU_TARGET_NAME} PRIVATE ${GMU_COMPILE_DEFINITIONS} )

    
#windows application
    set_target_properties(${GMU_TARGET_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    set_target_properties(${GMU_TARGET_NAME} PROPERTIES LINK_FLAGS_DEBUG "/SUBSYSTEM:WINDOWS")
    set_target_properties(${GMU_TARGET_NAME} PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS")
    set_target_properties(${GMU_TARGET_NAME} PROPERTIES RELWITHDEBINFO "/SUBSYSTEM:WINDOWS")
    set_target_properties(${GMU_TARGET_NAME} PROPERTIES MINSIZEREL "/SUBSYSTEM:WINDOWS")

ENDMACRO( GMU_WINDOWS_APP_BUILD )


# drop all "*T.cpp" files from list
macro (REMOVE_TEMPLATES list)
  foreach (_file ${${list}})
    if (_file MATCHES "T.cpp$")
      list (REMOVE_ITEM ${list} ${_file})
    endif ()
  endforeach ()
endmacro (REMOVE_TEMPLATES)