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


CONFIGURE_FILE("${CMAKE_CURRENT_SOURCE_DIR}/vtkVisItReaderExport.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/vtkVisItReaderExport.h")
               
         
ADD_PARAVIEW_PLUGIN(VisItReader "1.0"
  SERVER_MANAGER_SOURCES ${SERVER_SOURCES}
  SERVER_SOURCES ${EXTRA_SOURCES}
  SERVER_MANAGER_XML VisItReader.xml)
  
TARGET_LINK_LIBRARIES(VisItReader avtdatabase_ser visitcommon avtdbatts avtpipeline_ser visit_vtk)  
