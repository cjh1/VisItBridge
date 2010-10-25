#VISIT_INCLUDE_READER_SOURCE is located in Cmake/ViItCommon.cmake

PROJECT(vtkVisItDatabases)

#these are used by visit_include_reader
set(VISIT_DATABASE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
set(VISIT_SERVER_SOURCES CACHE INTERNAL "vtk classes to wrap for client server" FORCE)
set(VISIT_DB_SOURCES "" CACHE INTERNAL "visit sources for readers" FORCE) 
set(VISIT_DB_INC_DIRS CACHE INTERNAL "include directories" FORCE)

VISIT_INCLUDE_READER_SOURCE(Fluent)
VISIT_INCLUDE_READER_SOURCE(Miranda)
VISIT_INCLUDE_READER_SOURCE(ProteinDataBank)

if (PARAVIEW_HDF5_LIBRARIES)  
  VISIT_INCLUDE_READER_SOURCE(Enzo)
  VISIT_INCLUDE_READER_SOURCE(Pixie)  
  VISIT_INCLUDE_READER_SOURCE(SAMRAI)
  VISIT_INCLUDE_READER_SOURCE(Tetrad)
endif(PARAVIEW_HDF5_LIBRARIES)

if(HAVE_LIBSILO)
  VISIT_INCLUDE_READER_SOURCE(Silo)
endif(HAVE_LIBSILO)  

if ( HAVE_LIBNETCDF )
  #VISIT_INCLUDE_READER_SOURCE(NETCDF)
endif( HAVE_LIBNETCDF )

SET(KIT VisItDatabases)
SET(KIT_UPCASE "VISITDATABASES")

#include all the visit directories 
VISIT_READER_INCLUDES()
include_directories(
  ${VISIT_DB_INC_DIRS}
  )
  
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

if(HAVE_LIBSILO)
  target_link_libraries(vtk${KIT} ${SILO_LIBRARY})
endif(HAVE_LIBSILO)