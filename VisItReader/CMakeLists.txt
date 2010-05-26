PROJECT(VisItReader)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(SERVER_SOURCES
vtkVisItReader.h
vtkVisItReader.cxx
)

SET(EXTRA_SOURCES
avtVisItReaderFileFormat.h
avtVisItReaderFileFormat.C
VisItReaderCommonPluginInfo.C
VisItReaderEnginePluginInfo.C
VisItReaderMDServerPluginInfo.C
VisItReaderPluginInfo.h
VisItReaderPluginInfo.C
)
 
SET_SOURCE_FILES_PROPERTIES(
avtVisItReaderFileFormat.h
avtVisItReaderFileFormat.C
VisItReaderCommonPluginInfo.C
VisItReaderEnginePluginInfo.C
VisItReaderMDServerPluginInfo.C
VisItReaderPluginInfo.h
VisItReaderPluginInfo.C
WRAP_EXCLUDE)
               
INCLUDE_DIRECTORIES(
${AVT_DATABASE_SOURCE_DIR}/Database
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
${VTK_INCLUDE_DIRS}
)         
ADD_PARAVIEW_PLUGIN(VisItReader "1.0"
  SERVER_MANAGER_SOURCES ${SERVER_SOURCES}
  SERVER_SOURCES ${EXTRA_SOURCES}
  SERVER_MANAGER_XML VisItReader.xml
  GUI_RESOURCE_FILES VisItReaderGUI.xml)
  
TARGET_LINK_LIBRARIES(VisItReader avtdatabase_ser visitcommon avtdbatts avtpipeline_ser visit_vtk)  
