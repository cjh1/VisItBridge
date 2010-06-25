
MACRO(VISIT_INSTALL_TARGETS)    
  INSTALL(TARGETS ${ARGN}
    DESTINATION bin
    COMPONENT Runtime)              
ENDMACRO(VISIT_INSTALL_TARGETS)

MACRO(VISIT_PLUGIN_INCLUDES)
  include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${AVTALGORITHMS_BINARY_DIR}
    ${AVTALGORITHMS_SOURCE_DIR}
    ${VISIT_COMMON_INCLUDES}
    ${VISIT_SOURCE_DIR}/include
    ${AVT_DATABASE_SOURCE_DIR}/Database
    ${AVT_DATABASE_SOURCE_DIR}/Ghost
    ${AVT_DATABASE_SOURCE_DIR}/Formats
    ${AVT_DBATTS_SOURCE_DIR}/MetaData
    ${AVT_DBATTS_SOURCE_DIR}/SIL
    ${VISIT_SOURCE_DIR}/avt/Math
    ${VISIT_SOURCE_DIR}/avt/VisWindow/VisWindow
    ${AVT_PIPELINE_SOURCE_DIR}/AbstractFilters
    ${AVT_PIPELINE_SOURCE_DIR}/Data
    ${AVT_PIPELINE_SOURCE_DIR}/Pipeline
    ${AVT_PIPELINE_SOURCE_DIR}/Sinks
    ${AVT_PIPELINE_SOURCE_DIR}/Sources
    ${VISIT_SOURCE_DIR}/visit_vtk/full
    ${VISIT_SOURCE_DIR}/visit_vtk/lightweight
    ${BOOST_INCLUDE_DIR}
    ${VTK_INCLUDE_DIRS}
    )
ENDMACRO(VISIT_PLUGIN_INCLUDES)

FUNCTION(ADD_VISIT_READER NAME VERSION)
set(PLUGIN_NAME "vtk${NAME}")
set(PLUGIN_VERSION "${VERSION}")
set(ARG_VISIT_READER_NAME)
set(ARG_VISIT_INCLUDE_NAME)
set(ARG_VISIT_READER_TYPE)
set(ARG_VISIT_READER_FILE_PATTERN)
set(ARG_VISIT_READER_USES_OPTIONS OFF)
set(ARG_SERVER_SOURCES)

PV_PLUGIN_PARSE_ARGUMENTS(ARG 
  "VISIT_READER_NAME;VISIT_INCLUDE_NAME;VISIT_READER_TYPE;VISIT_READER_FILE_PATTERN;VISIT_READER_USES_OPTIONS;SERVER_SOURCES"
    "" ${ARGN} )    
#check reader types
string(REGEX MATCH "^[SM]T[SM]D$" VALID_READER_TYPE ${ARG_VISIT_READER_TYPE})

if (!VALID_READER_TYPE)
  MESSAGE(FATAL_ERROR "Invalid Reader Type. Valid Types are STSD, STMD, MTSD, MTMD")
endif()

#if the user hasn't defined an include name, we presume the reader name
#is also the include name
if(NOT ARG_VISIT_INCLUDE_NAME)
  set(ARG_VISIT_INCLUDE_NAME ${ARG_VISIT_READER_NAME})
endif()

MESSAGE(STATUS "Generating wrappings for ${PLUGIN_NAME}")
VISIT_PLUGIN_INCLUDES()

if(ARG_VISIT_READER_USES_OPTIONS)
  #determine the name of the plugin info class by removing the 
  #avt from the start and the FileFormat from the back
  string(REGEX REPLACE "^avt|FileFormat$" "" TEMP_NAME ${ARG_VISIT_READER_NAME})
  set(ARG_VISIT_PLUGIN_INFO_HEADER ${TEMP_NAME}PluginInfo)
  set(ARG_VISIT_PLUGIN_INFO_CLASS ${TEMP_NAME}CommonPluginInfo)
endif()

set(XML_NAME ${NAME})
#need to generate the VTK class wrapper
string(SUBSTRING ${ARG_VISIT_READER_TYPE} 0 2 READER_WRAPPER_TYPE)
configure_file(
    ${VISIT_CMAKE_DIR}/VisItExport.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}Export.h @ONLY)      
configure_file(
    ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.h @ONLY)  
configure_file(
    ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}.cxx.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.cxx @ONLY)
  
#generate server manager xml file  
configure_file(
  ${VISIT_CMAKE_DIR}/VisItSM.xml.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}SM.xml @ONLY)

#generate reader xml 
configure_file(
  ${VISIT_CMAKE_DIR}/VisItGUI.xml.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}GUI.xml @ONLY)  
  
  
set(reader_sources
  ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.h
    )  
set(reader_server_xml
  ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}SM.xml
  )
set(reader_client_xml
  ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}GUI.xml
  )

#add the vtk classes to the argument list
set(PV_ARGS ${ARGN})
list(APPEND PV_ARGS "SERVER_MANAGER_SOURCES;${reader_sources}")

#now we need to add the XML info
list(APPEND PV_ARGS "SERVER_MANAGER_XML;${reader_server_xml}")
list(APPEND PV_ARGS "GUI_RESOURCE_FILES;${reader_client_xml}")

ADD_PARAVIEW_PLUGIN( ${NAME} ${VERSION} ${PV_ARGS} )
ENDFUNCTION(ADD_VISIT_READER)

FUNCTION(ADD_VISIT_INTERFACE_READER NAME VERSION)

set(INTERFACE_NAME "vtk${NAME}")
set(INTERFACE_VERSION "${VERSION}")
set(ARG_VISIT_READER_NAMES)
set(ARG_VISIT_READER_TYPES)
set(ARG_VISIT_READER_INCLUDES)
set(ARG_VISIT_READER_FILE_PATTERN)
set(ARG_SERVER_SOURCES)

PV_PLUGIN_PARSE_ARGUMENTS(ARG 
  "VISIT_READER_NAMES;VISIT_READER_TYPES;VISIT_READER_INCLUDES;VISIT_READER_FILE_PATTERN;SERVER_SOURCES"
    "" ${ARGN} )   
    
message(STATUS "Generating wrappings for ${INTERFACE_NAME}")    
VISIT_PLUGIN_INCLUDES()    

#check reader types
set(INTERFACE_SOURCES)
set(INTERFACE_SMXML)
set(INTERFACE_GUIXML)
list(LENGTH ARG_VISIT_READER_NAMES NUM_READERS)
foreach( index RANGE ${NUM_READERS})
  if ( index LESS NUM_READERS )
    list(GET ARG_VISIT_READER_NAMES ${index} ARG_VISIT_READER_NAME)
    list(GET ARG_VISIT_READER_TYPES ${index} ARG_VISIT_READER_TYPE)
    list(GET ARG_VISIT_READER_INCLUDES ${index} ARG_VISIT_INCLUDE_NAME)
    
    #need to set up the vars needed by the configures
    string(REGEX REPLACE "^avt|FileFormat$" "" TEMP_NAME ${ARG_VISIT_READER_NAME})
    set(PLUGIN_NAME "vtk${TEMP_NAME}Reader")
    set(XML_NAME "VisIt${TEMP_NAME}Reader")
    
    #need to generate the VTK class wrapper
    string(SUBSTRING ${ARG_VISIT_READER_TYPE} 0 2 READER_WRAPPER_TYPE)
    configure_file(
        ${VISIT_CMAKE_DIR}/VisItExport.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}Export.h @ONLY)      
    configure_file(
        ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.h @ONLY)  
    configure_file(
        ${VISIT_CMAKE_DIR}/VisIt${READER_WRAPPER_TYPE}.cxx.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.cxx @ONLY)
      
    #generate server manager xml file  
    configure_file(
      ${VISIT_CMAKE_DIR}/VisItSM.xml.in
      ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}SM.xml @ONLY)

    #generate reader xml 
    configure_file(
      ${VISIT_CMAKE_DIR}/VisItGUI.xml.in
      ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}GUI.xml @ONLY)  
  
    LIST(APPEND INTERFACE_SOURCES 
      ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.cxx
      ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}.h
      )
    LIST(APPEND INTERFACE_SMXML 
      ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}SM.xml
      )
    LIST(APPEND INTERFACE_GUIXML 
      ${CMAKE_CURRENT_BINARY_DIR}/${PLUGIN_NAME}GUI.xml
      )
    
  endif()
endforeach( index )    

#add the vtk classes to the argument list
set(PV_ARGS ${ARGN})
list(APPEND PV_ARGS "SERVER_MANAGER_SOURCES;${INTERFACE_SOURCES}")

#now we need to add the XML info
list(APPEND PV_ARGS "SERVER_MANAGER_XML;${INTERFACE_SMXML}")
list(APPEND PV_ARGS "GUI_RESOURCE_FILES;${INTERFACE_GUIXML}")

ADD_PARAVIEW_PLUGIN( ${NAME} ${VERSION} ${PV_ARGS} )

ENDFUNCTION(ADD_VISIT_INTERFACE_READER)
