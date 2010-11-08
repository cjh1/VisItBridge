
set(SOURCES
  avtVsFileFormat.C
  VsFilter.C
  VsH5Reader.C
  VsSchema.C
  VsUtils.C
  )
ADD_VISIT_READER(VisItVsReader "1.0"
  VISIT_READER_TYPE "STSD"
  VISIT_READER_NAME "avtVsFileFormat"    
  SERVER_SOURCES ${SOURCES}
  )
