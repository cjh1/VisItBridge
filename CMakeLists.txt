cmake_minimum_required(VERSION 2.6)

project(VisItBridgePlugin)
FIND_PACKAGE(ParaView REQUIRED)

IF (COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
ENDIF (COMMAND cmake_policy)

#set up some vars we need to compile
set(BUILD_SHARED_LIBS ${PARAVIEW_BUILD_SHARED_LIBS})
set(VISIT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(VISIT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(VISIT_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake )

include(CTest)
include(${VISIT_CMAKE_DIR}/VisItCommon.cmake)

#lower warning level since this project has to many warnings
set(OLD_CMAKE_CXX_WARNING_LEVEL ${CMAKE_CXX_WARNING_LEVEL})
set(CMAKE_CXX_WARNING_LEVEL 1)

#-----------------------------------------------------------------------------
# Setup Vars for visit-config.h
#-----------------------------------------------------------------------------

set(VISIT_DBIO_ONLY ON) #Build only visitconvert and engine plugins
if(VISIT_DBIO_ONLY)
    add_definitions(-DDBIO_ONLY)    
endif(VISIT_DBIO_ONLY)   

#Check to see if ofstreams rdbuf is public. If it is NOT public set NO_SETBUF
TRY_COMPILE(VISIT_COMPILER_FSTREAM_WORKAROUND
        ${CMAKE_CURRENT_BINARY_DIR}/CMakeTmp
        ${VISIT_CMAKE_DIR}/testFStream.cxx)
IF (NOT VISIT_COMPILER_FSTREAM_WORKAROUND)
   set(NO_SETBUF 1)
ENDIF (NOT VISIT_COMPILER_FSTREAM_WORKAROUND)  

#Setup the version since some files need this
SET(VISIT_VERSION "2.0.0")
    
# Set the slash characters based on the platform
IF(WIN32)
    SET(VISIT_SLASH_CHAR   "'\\\\'")
    SET(VISIT_SLASH_STRING "\"\\\\\"")
ELSE(WIN32)
    SET(VISIT_SLASH_CHAR   "'/'")
    SET(VISIT_SLASH_STRING "\"/\"")
ENDIF(WIN32)

# Check for plugin extension
IF(VISIT_STATIC)
    SET(VISIT_PLUGIN_EXTENSION   ".a")
ELSE(VISIT_STATIC)
    IF(WIN32)
        SET(VISIT_PLUGIN_EXTENSION   ".dll")
    ELSE(WIN32)
        IF(APPLE)
            SET(VISIT_PLUGIN_EXTENSION   ".dylib")
        ELSE(APPLE)
            SET(VISIT_PLUGIN_EXTENSION   ".so")
        ENDIF(APPLE)
    ENDIF(WIN32)
ENDIF(VISIT_STATIC)

#Get the size of types
INCLUDE(CMakeBackwardCompatibilityC)
INCLUDE(CheckTypeSize)
# Check for type sizes, endian
SET(SIZEOF_BOOLEAN              ${CMAKE_SIZEOF_BOOLEAN})
SET(SIZEOF_CHAR                 ${CMAKE_SIZEOF_CHAR})
SET(SIZEOF_DOUBLE               ${CMAKE_SIZEOF_DOUBLE})
SET(SIZEOF_FLOAT                ${CMAKE_SIZEOF_FLOAT})
SET(SIZEOF_INT                  ${CMAKE_SIZEOF_INT})
SET(SIZEOF_LONG                 ${CMAKE_SIZEOF_LONG})
SET(SIZEOF_LONG_DOUBLE          ${CMAKE_SIZEOF_LONG_DOUBLE})
SET(SIZEOF_LONG_FLOAT           ${CMAKE_SIZEOF_LONG_FLOAT})
SET(SIZEOF_LONG_LONG            ${CMAKE_SIZEOF_LONG_LONG})
SET(SIZEOF_SHORT                ${CMAKE_SIZEOF_SHORT})
SET(SIZEOF_UNSIGNED_CHAR        ${CMAKE_SIZEOF_UNSIGNED_CHAR})
SET(SIZEOF_UNSIGNED_INT         ${CMAKE_SIZEOF_UNSIGNED_INT})
SET(SIZEOF_UNSIGNED_LONG        ${CMAKE_SIZEOF_UNSIGNED_LONG})
SET(SIZEOF_UNSIGNED_LONG_LONG   ${CMAKE_SIZEOF_UNSIGNED_LONG_LONG})
SET(SIZEOF_UNSIGNED_SHORT       ${CMAKE_SIZEOF_UNSIGNED_SHORT})
SET(SIZEOF_VOID_P               ${CMAKE_SIZEOF_VOID_P})
#SET(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE)
CHECK_TYPE_SIZE("off64_t" SIZEOF_OFF64_T)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)

#-----------------------------------------------------------------------------
# Create visit-config.h
#-----------------------------------------------------------------------------

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/visit-cmake.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/visit-config.h @ONLY IMMEDIATE)    


# Set up easy to use includes for the common directory
set(VISIT_COMMON_INCLUDES
    ${CMAKE_CURRENT_BINARY_DIR}  
    ${CMAKE_CURRENT_SOURCE_DIR}/common/Exceptions/Database
    ${CMAKE_CURRENT_SOURCE_DIR}/common/Exceptions/Pipeline
    ${CMAKE_CURRENT_SOURCE_DIR}/common/Exceptions/Plotter
    ${CMAKE_CURRENT_SOURCE_DIR}/common/comm
    ${CMAKE_CURRENT_SOURCE_DIR}/common/expr
    ${CMAKE_CURRENT_SOURCE_DIR}/common/icons
    ${CMAKE_CURRENT_SOURCE_DIR}/common/misc  
    ${CMAKE_CURRENT_SOURCE_DIR}/common/parser
    ${CMAKE_CURRENT_SOURCE_DIR}/common/plugin
    ${CMAKE_CURRENT_SOURCE_DIR}/common/proxybase
    ${CMAKE_CURRENT_SOURCE_DIR}/common/state
    ${CMAKE_CURRENT_SOURCE_DIR}/common/utility
    ${CMAKE_CURRENT_SOURCE_DIR}/common/common/misc
    ${CMAKE_CURRENT_SOURCE_DIR}/common/common/plugin
    ${CMAKE_CURRENT_SOURCE_DIR}/common/common/state
    ${CMAKE_CURRENT_SOURCE_DIR}/common/common/utility
    )
    
#watch out, you need to make sure common/parser is always in front of
# python2.X includes

INCLUDE_DIRECTORIES(
  ${VISIT_COMMON_INCLUDES}
  )
INCLUDE(${PARAVIEW_USE_FILE})  
INCLUDE_DIRECTORIES(
  ${VTK_INCLUDE_DIRS}
  ${PARAVIEW_INCLUDE_DIRS}
  ${PARAVIEW_GUI_INCLUDE_DIRS}   
  )    
add_subdirectory(common)
add_subdirectory(visit_vtk)
add_subdirectory(avt)
add_subdirectory(VisItReader)


SET(CMAKE_CXX_WARNING_LEVEL ${OLD_CMAKE_CXX_WARNING_LEVEL})

