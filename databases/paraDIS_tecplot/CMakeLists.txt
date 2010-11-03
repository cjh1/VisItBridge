
set(SOURCES
  avtparaDIS_tecplotFileFormat.C
  avtparaDIS_tecplotFileFormat.h  
  RC_c_lib/args.c 
  RC_c_lib/debugutil.c 
  RC_c_lib/signals.c 
  RC_c_lib/fileutils.c 
  RC_c_lib/inventor.c
  RC_cpp_lib/RangeList.C 
  RC_cpp_lib/timer.C 
  RC_cpp_lib/pathutil.C
)

ADD_VISIT_READER(VisItparaDIS_tecplotReader "1.0" 
  VISIT_READER_TYPE "STSD"
  VISIT_READER_NAME "avtparaDIS_tecplotFileFormat"     
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )
