PROJECT(VisItFluentReader)

SET(SOURCES
avtFluentFileFormat.h
avtFluentFileFormat.C
FluentCommonPluginInfo.C
FluentEnginePluginInfo.C
FluentMDServerPluginInfo.C
FluentPluginInfo.h
FluentPluginInfo.C
)

ADD_VISIT_READER(VisItFluentReader "1.0"  
  VISIT_READER_TYPE "STMD"
  VISIT_READER_NAME "avtFluentFileFormat"  
  VISIT_READER_FILE_PATTERN "cas"
  SERVER_SOURCES ${SOURCES}  
  )

TARGET_LINK_LIBRARIES(VisItFluentReader AVTAlgorithms)