
set(SOURCES
  avtShapefileFileFormat.C
  avtShapefileFileFormat.h
  avtShapefileOptions.C
  avtShapefileOptions.h
  ShapefilePluginInfo.h
  ShapefilePluginInfo.C
  ShapefileEnginePluginInfo.C
  ShapefileMDServerPluginInfo.C
  ShapefileCommonPluginInfo.C
  esriShapefile.C
  dbfFile.C
)

ADD_VISIT_READER(VisItShapefileReader "1.0" 
  VISIT_READER_TYPE "STSD"
  VISIT_READER_NAME "avtShapefileFileFormat"     
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )
