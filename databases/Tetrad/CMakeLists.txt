PROJECT(VisITTetradReader)

SET(SOURCES
avtTetradFileFormat.h
avtTetradFileFormat.C
)

ADD_VISIT_READER(VisITetradReader "1.0"  
  VISIT_READER_TYPE "MTSD"
  VISIT_READER_NAME "avtTetradFileFormat"    
  SERVER_SOURCES ${SOURCES}  
  )

TARGET_LINK_LIBRARIES(VisITetradReader ${PARAVIEW_HDF5_LIBRARIES} vtkVisItAVTAlgorithms)