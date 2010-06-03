PROJECT(VisItEnzoReader)

SET(SOURCES
avtEnzoFileFormat.h
avtEnzoFileFormat.C
)

ADD_VISIT_STMDREADER(VisItEnzoReader "1.0"  
  VISIT_READER_NAME "avtEnzoFileFormat"  
  VISIT_READER_FILE_PATTERN "hierarchy boundary"
  SERVER_SOURCES ${SOURCES}  
  )

TARGET_LINK_LIBRARIES(VisItEnzoReader ${PARAVIEW_HDF5_LIBRARIES} AVTAlgorithms)