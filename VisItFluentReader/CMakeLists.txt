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
${CMAKE_CURRENT_SOURCE_DIR}
${VISIT_COMMON_INCLUDES}
${VISIT_INCLUDE_DIR}/avt/DBAtts/MetaData
${VISIT_INCLUDE_DIR}/avt/DBAtts/SIL
${VISIT_INCLUDE_DIR}/avt/Database/Database
${VISIT_INCLUDE_DIR}/avt/Database/Formats
${VISIT_INCLUDE_DIR}/avt/Database/Ghost
${VISIT_INCLUDE_DIR}/avt/FileWriter
${VISIT_INCLUDE_DIR}/avt/Filters
${VISIT_INCLUDE_DIR}/avt/Math
${VISIT_INCLUDE_DIR}/avt/Pipeline/AbstractFilters
${VISIT_INCLUDE_DIR}/avt/Pipeline/Data
${VISIT_INCLUDE_DIR}/avt/Pipeline/Pipeline
${VISIT_INCLUDE_DIR}/avt/Pipeline/Sinks
${VISIT_INCLUDE_DIR}/avt/Pipeline/Sources
${VISIT_INCLUDE_DIR}/avt/VisWindow/VisWindow
${VISIT_INCLUDE_DIR}/visit_vtk/full
${VISIT_INCLUDE_DIR}/visit_vtk/lightweight
${VTK_INCLUDE_DIRS} 
)

LINK_DIRECTORIES(${VISIT_LIBRARY_DIR} ${VTK_LIBRARY_DIRS})

ADD_PARAVIEW_PLUGIN(VisItFluentReader "1.0"
  SERVER_MANAGER_SOURCES ${SERVER_SOURCES}
  SERVER_SOURCES ${EXTRA_SOURCES}
  SERVER_MANAGER_XML VisItFluentReaderSM.xml
  GUI_RESOURCE_FILES VisItFluentReaderGUI.xml)
  
TARGET_LINK_LIBRARIES(VisItFluentReader visitcommon avtdatabase_ser avtpipeline_ser)  
