
set(SOURCES
  avtTecplotFileFormat.C
  avtTecplotBinaryFileFormat.C
  TecplotFile.C
  )
ADD_VISIT_READER(VisItTecplotReader "1.0"
  VISIT_READER_TYPE "STMD"
  VISIT_READER_NAME "avtTecplotFileFormat"    
  SERVER_SOURCES ${SOURCES}
  )
