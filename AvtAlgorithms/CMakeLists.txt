PROJECT(vtkVisItAVTAlgorithms)

SET(SOURCES
vtkAvtFileFormatAlgorithm.h
vtkAvtFileFormatAlgorithm.cxx
vtkAvtMTSDFileFormatAlgorithm.h
vtkAvtMTSDFileFormatAlgorithm.cxx
vtkAvtMTMDFileFormatAlgorithm.h
vtkAvtMTMDFileFormatAlgorithm.cxx
vtkAvtSTSDFileFormatAlgorithm.h
vtkAvtSTSDFileFormatAlgorithm.cxx
vtkAvtSTMDFileFormatAlgorithm.h
vtkAvtSTMDFileFormatAlgorithm.cxx
)
#########################################################################
SET (dynamicHeaders "${dynamicHeaders};${PVFilters_BINARY_DIR}/vtkPVFiltersConfig.h")
CONFIGURE_FILE(${PVFilters_SOURCE_DIR}/vtkPVFiltersConfig.h.in
  ${PVFilters_BINARY_DIR}/vtkPVFiltersConfig.h
  ESCAPE_QUOTES)
  
CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/vtkAvtAlgorithmsExport.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/vtkAvtAlgorithmsExport.h")
 set(dynamicHeaders "$dynamicHeaders;${CMAKE_CURRENT_BINARY_DIR}/vtkAvtAlgorithmsExport.h")

INCLUDE_DIRECTORIES(
${CMAKE_CURRENT_SOURCE_DIR}
${CMAKE_CURRENT_BINARY_DIR}
${AVT_DATABASE_SOURCE_DIR}/Database
${AVT_DATABASE_SOURCE_DIR}/Ghost
${AVT_DATABASE_SOURCE_DIR}/Formats
${VISIT_COMMON_INCLUDES}
${AVT_DBATTS_SOURCE_DIR}/MetaData
${AVT_DBATTS_SOURCE_DIR}/SIL
${VISIT_SOURCE_DIR}/avt/Math
${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
${AVT_PIPELINE_SOURCE_DIR}/AbstractFilters
${AVT_PIPELINE_SOURCE_DIR}/Data
${AVT_PIPELINE_SOURCE_DIR}/Pipeline
${AVT_PIPELINE_SOURCE_DIR}/Sinks
${AVT_PIPELINE_SOURCE_DIR}/Sources
${VISIT_SOURCE_DIR}/visit_vtk/full
${VISIT_SOURCE_DIR}/visit_vtk/lightweight
${VTK_INCLUDE_DIRS}
)
ADD_LIBRARY(vtkVisItAVTAlgorithms ${SOURCES})
TARGET_LINK_LIBRARIES(vtkVisItAVTAlgorithms visitcommon avtdatabase_ser avtpipeline_ser visit_vtk vtkPVServerManager)

#Install
#-------------------------------------------------------------------------------------------------
VISIT_INSTALL_TARGETS(vtkVisItAVTAlgorithms "${CMAKE_CURRENT_BINARY_DIR}/vtkAvtAlgorithmsExport.h")