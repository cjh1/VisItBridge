MACRO(VISIT_INCLUDE_READER_SOURCE name state)
option(VISIT_BUILD_READER_${name} "Build VisIt Reader ${name}" ${state})
if (VISIT_BUILD_READER_${name})
  ADD_SUBDIRECTORY(${name})
endif()
ENDMACRO(VISIT_INCLUDE_READER_SOURCE name)

PROJECT(vtkVisItDatabases)

#these are used by visit_include_reader
set(VISIT_DATABASE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(VISIT_SERVER_SOURCES CACHE INTERNAL "vtk classes to wrap for client server" FORCE)
set(VISIT_DB_SOURCES "" CACHE INTERNAL "visit sources for readers" FORCE) 
set(VISIT_DB_INC_DIRS CACHE INTERNAL "include directories" FORCE)

set(DEFAULT_BRIDGE_READERS
  ANALYZE
  ANSYS
  AugDecomp
  AUXFile
  BOV
  CEAucd
  Chombo  
  CMAT
  CTRL
  Curve2D
  DDCMD
  Dyna3D
  Enzo
  ExtrudedVol
  FLASH  
  Fluent
  Gadget
  GGCM
  GTC
  GULP
  Miranda
  Pixie
  ProteinDataBank
  SAMRAI
  Tetrad
  )
if(NOT WIN32)
  list(APPEND DEFAULT_BRIDGE_READERS
    Claw
    )
endif(NOT WIN32)    
    
#add all the default readers to the library    
foreach(db ${DEFAULT_BRIDGE_READERS})
  add_subdirectory(${db})
endforeach(db)

  
VISIT_INCLUDE_READER_SOURCE(Silo OFF)
if(VISIT_BUILD_READER_Silo)
  if(NOT HAVE_LIBSILO)
    #if we currently don't have silo setup make it required
    find_package(SILO REQUIRED)
  endif(NOT HAVE_LIBSILO)
  if(NOT SILO_FOUND)
    message(FATAL_ERROR "Unable to locate Silo library.")
  endif(NOT SILO_FOUND)    
endif(VISIT_BUILD_READER_Silo)


VISIT_INCLUDE_READER_SOURCE(CGNS OFF)
if(VISIT_BUILD_READER_CGNS)
  if(NOT HAVE_LIBCGNS)
    #if we currently don't have silo setup make it required
    find_package(CGNS REQUIRED)
  endif(NOT HAVE_LIBCGNS)
  if(NOT CGNS_FOUND)
    message(FATAL_ERROR "Unable to locate CGNS library.")
  endif(NOT CGNS_FOUND)  
endif(VISIT_BUILD_READER_CGNS)

#currently needs more work
#if ( HAVE_LIBNETCDF )
  #VISIT_INCLUDE_READER_SOURCE(NETCDF)
#endif( HAVE_LIBNETCDF )

SET(KIT VisItDatabases)
SET(KIT_UPCASE "VISITDATABASES")

#include all the visit directories 
include_directories(
  ${VISITBRIDGE_USE_FILE}
  )

include_directories(
  ${VISIT_DB_INC_DIRS}
  )


if(HAVE_LIBSILO AND VISIT_BUILD_READER_Silo)
  include_directories(${SILO_INCLUDE_DIR})
endif(HAVE_LIBSILO AND VISIT_BUILD_READER_Silo)

if(HAVE_LIBCGNS AND VISIT_BUILD_READER_CGNS)
  include_directories(${CGNS_INCLUDE_DIR})
endif(HAVE_LIBCGNS AND VISIT_BUILD_READER_CGNS)
  
SET(Kit_SRCS
  ${VISIT_SERVER_SOURCES}
  )

SET_SOURCE_FILES_PROPERTIES(
  ${VISIT_DB_SOURCES}
  WRAP_EXCLUDE
  )

  
#setup the kit so we get client server bindings  
SET(VTK_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
INCLUDE(${VTK_CMAKE_DIR}/vtkExportKit.cmake)
VTK_EXPORT_KIT("${KIT}" "${KIT_UPCASE}" "${Kit_SRCS}")
  
#build all the readers into a single library
add_library(vtk${KIT} ${Kit_SRCS} ${VISIT_DB_SOURCES})    

target_link_libraries(vtk${KIT} vtkVisItAVTAlgorithms) 
if(PARAVIEW_HDF5_LIBRARIES)
  target_link_libraries(vtk${KIT} ${PARAVIEW_HDF5_LIBRARIES})
endif(PARAVIEW_HDF5_LIBRARIES)  

if(HAVE_LIBNETCDF)
  target_link_libraries(vtk${KIT} vtkNetCDF)
endif(HAVE_LIBNETCDF)

if(HAVE_LIBSILO AND VISIT_BUILD_READER_Silo)
  target_link_libraries(vtk${KIT} ${SILO_LIBRARY})
endif(HAVE_LIBSILO AND VISIT_BUILD_READER_Silo)

if(HAVE_LIBCGNS AND VISIT_BUILD_READER_CGNS)  
  target_link_libraries(vtk${KIT} ${CGNS_LIBRARY})
endif(HAVE_LIBCGNS AND VISIT_BUILD_READER_CGNS)

#Install
#-------------------------------------------------------------------------------------------------
VISIT_INSTALL_TARGETS(vtk${KIT})
