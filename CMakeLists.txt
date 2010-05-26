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

set(VISIT_PARALLEL ${VTK_USE_MPI})
IF(VTK_USE_MPI)
	INCLUDE(FindMPI)
  INCLUDE_DIRECTORIES(  
    ${MPI_INCLUDE_PATH}
    )  
ENDIF(VTK_USE_MPI)


include(CTest)
include(${VISIT_CMAKE_DIR}/VisItCommon.cmake)

#block out most of the warnings in visit on windows
IF (WIN32)
  ADD_DEFINITIONS(-D_USE_MATH_DEFINES)
  IF ("${VISIT_MSVC_VERSION}" STREQUAL "MSVC80")
      ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
      ADD_DEFINITIONS(-D_SCL_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
      ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
  ENDIF ("${VISIT_MSVC_VERSION}" STREQUAL "MSVC80")
ENDIF(WIN32)  

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

#-----------------------------------------------------------------------------
# Detect packages here. We could probably write macros that we can include from
# elsewhere for this. 
#-----------------------------------------------------------------------------
INCLUDE(CheckIncludeFiles)
INCLUDE(CMakeBackwardCompatibilityC)
INCLUDE(CMakeBackwardCompatibilityCXX)
INCLUDE(CheckTypeSize)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckSymbolExists)
INCLUDE(TestBigEndian)
INCLUDE(FindOpenGL)

CHECK_INCLUDE_FILES (fcntl.h     HAVE_FCNTL_H)
CHECK_INCLUDE_FILES (inttypes.h  HAVE_INTTYPES_H)
CHECK_INCLUDE_FILES (malloc.h    HAVE_MALLOC_H)
CHECK_INCLUDE_FILES (limits.h    HAVE_LIMITS_H)
CHECK_INCLUDE_FILES (memory.h    HAVE_MEMORY_H)
CHECK_INCLUDE_FILES (stdint.h    HAVE_STDINT_H)
CHECK_INCLUDE_FILES (stdlib.h    HAVE_STDLIB_H)
CHECK_INCLUDE_FILES (strings.h   HAVE_STRINGS_H)
CHECK_INCLUDE_FILES (string.h    HAVE_STRING_H)
CHECK_INCLUDE_FILES (sys/time.h  HAVE_SYS_TIME_H)
CHECK_INCLUDE_FILES (sys/types.h HAVE_SYS_TYPES_H)
CHECK_INCLUDE_FILES (sys/stat.h  HAVE_SYS_STAT_H)
CHECK_INCLUDE_FILES (unistd.h    HAVE_UNISTD_H)
CHECK_INCLUDE_FILES (zlib.h      HAVE_ZLIB_H)
CHECK_INCLUDE_FILES (stdbool.h   HAVE_STDBOOL_H)

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
SET(CMAKE_REQUIRED_DEFINITIONS -D_LARGEFILE64_SOURCE)
CHECK_TYPE_SIZE("off64_t" SIZEOF_OFF64_T)
TEST_BIG_ENDIAN(WORDS_BIGENDIAN)


# manually check for socklen_t as CHECK_SYMBOL_EXISTS
# doesn't appear to work on linux (at least) 
IF ("HAVE_SOCKLEN_T" MATCHES "^HAVE_SOCKLEN_T$")
    MESSAGE(STATUS "Check for socklen_t")
    TRY_COMPILE(HAVE_SOCKLEN_T
        ${CMAKE_CURRENT_BINARY_DIR}
        ${VISIT_SOURCE_DIR}/CMake/TestSocklenT.c
        OUTPUT_VARIABLE SLT
    )
    IF (HAVE_SOCKLEN_T)
        SET(HAVE_SOCKLEN_T 1 CACHE INTERNAL "support for socklen_t")
        MESSAGE(STATUS "Check for socklen_t - found")
    ELSE(HAVE_SOCKLEN_T)
        SET(HAVE_SOCKLEN_T 0 CACHE INTERNAL "support for socklen_t")
        MESSAGE(STATUS "Check for socklen_t - not found")
    ENDIF (HAVE_SOCKLEN_T)
ENDIF ("HAVE_SOCKLEN_T" MATCHES "^HAVE_SOCKLEN_T$")


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

#grab the third party macros from vtk, also have to recreate the VTK_BINARY_DIR variable
SET(VTK_BINARY_DIR ${PARAVIEW_VTK_DIR} )
INCLUDE(${VTK_SOURCE_DIR}/Cmake/vtkThirdParty.cmake)

#setup ZLib
VTK_THIRD_PARTY_INCLUDE(ZLIB vtkzlib)  
VTK_THIRD_PARTY_INCLUDE(PNG vtkpng)
INCLUDE_DIRECTORIES(
  ${VTK_INCLUDE_DIRS}
  ${PARAVIEW_INCLUDE_DIRS}
  ${PARAVIEW_GUI_INCLUDE_DIRS}   
  ${VTK_INCLUDE_DIRS_SOURCE_TREE}
  ) 
  
add_subdirectory(common)
add_subdirectory(visit_vtk)
add_subdirectory(avt)
add_subdirectory(AvtAlgorithms)
add_subdirectory(VisItReader)
add_subdirectory(VisItFluentReader)

