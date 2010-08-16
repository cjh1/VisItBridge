PROJECT(VisItEnzoReader)

SET(SOURCES
avtEnzoFileFormat.h
avtEnzoFileFormat.C
)

ADD_VISIT_READER(VisItEnzoReader "1.0"  
  VISIT_READER_TYPE "STMD"
  VISIT_READER_NAME "avtEnzoFileFormat"    
  SERVER_SOURCES ${SOURCES}    
  ) 

TARGET_LINK_LIBRARIES(VisItEnzoReader ${PARAVIEW_HDF5_LIBRARIES} vtkVisItAVTAlgorithms)  