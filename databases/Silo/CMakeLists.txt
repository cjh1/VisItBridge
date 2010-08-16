PROJECT(VisItSilo)

SET(SOURCES
avtSiloWriter.C
avtSiloFileFormat.C
avtSiloFileFormat.h
avtSiloOptions.C
avtSiloOptions.h
SiloPluginInfo.C
SiloPluginInfo.h
SiloCommonPluginInfo.C
SiloEnginePluginInfo.C
SiloMDServerPluginInfo.C
)

ADD_VISIT_READER(VisItSilo "1.0" 
  VISIT_READER_TYPE "STMD"
  VISIT_READER_NAME "avtSiloFileFormat"   
  VISIT_READER_FILE_PATTERN "silo pdb"
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )

TARGET_LINK_LIBRARIES(VisItSilo vtkVisItAVTAlgorithms ${SILO_LIBRARY})

