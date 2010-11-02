
SET(SOURCES
avtAugDecompFileFormat.C
)

ADD_VISIT_READER(VisItAugDecompReader "1.0"
  VISIT_READER_TYPE "MTMD"
  VISIT_READER_NAME "avtAugDecompFileFormat"    
  SERVER_SOURCES ${SOURCES}
  )
