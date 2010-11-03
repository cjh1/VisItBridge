
set(VISIT_DB_INC_DIRS
  ${VISIT_DB_INC_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/paraDIS_lib
  ${CMAKE_CURRENT_SOURCE_DIR}/RC_c_lib
  ${CMAKE_CURRENT_SOURCE_DIR}/RC_cpp_lib
  )
  
set(SOURCES
  avtparaDISFileFormat.C
  avtparaDISOptions.C
  paraDIS_lib/paradis.C
  paraDIS_lib/paradis_c_interface.C
  RC_c_lib/args.c
  RC_c_lib/debugutil.c
  RC_c_lib/fileutils.c
  RC_c_lib/inventor.c
  RC_cpp_lib/RangeList.C
  RC_cpp_lib/timer.C
  RC_cpp_lib/pathutil.C
  paraDISPluginInfo.h
  paraDISPluginInfo.C
  paraDISEnginePluginInfo.C
  paraDISMDServerPluginInfo.C
  paraDISCommonPluginInfo.C
)

ADD_VISIT_READER(VisItparaDISReader "1.0" 
  VISIT_READER_TYPE "STSD"
  VISIT_READER_NAME "avtparaDISFileFormat"     
  VISIT_READER_USES_OPTIONS ON
  SERVER_SOURCES ${SOURCES}
  )

