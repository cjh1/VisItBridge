PROJECT(VisItFluentReader)

SET(SERVER_SOURCES
vtkVisItFluentReader.h
vtkVisItFluentReader.cxx
)

SET(EXTRA_SOURCES
avtFluentFileFormat.h
avtFluentFileFormat.C
FluentCommonPluginInfo.C
FluentEnginePluginInfo.C
FluentMDServerPluginInfo.C
FluentPluginInfo.h
FluentPluginInfo.C
)

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

LINK_DIRECTORIES(${VISIT_LIBRARY_DIR} ${VTK_LIBRARY_DIRS})

ADD_PARAVIEW_PLUGIN(VisItFluentReader "1.0"
  SERVER_MANAGER_SOURCES ${SERVER_SOURCES}
  SERVER_SOURCES ${EXTRA_SOURCES}
  SERVER_MANAGER_XML VisItFluentReaderSM.xml
  GUI_RESOURCE_FILES VisItFluentReaderGUI.xml)
  
TARGET_LINK_LIBRARIES(VisItFluentReader visitcommon avtdatabase_ser avtpipeline_ser)  
