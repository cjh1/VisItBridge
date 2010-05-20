
MACRO(VISIT_INSTALL_TARGETS)
    IF( NOT BUILD_SHARED_LIBS)
        # Skip installation of static libraries when we build statically
        FOREACH(T ${ARGN})
            GET_TARGET_PROPERTY(pType ${T} TYPE)
            IF(NOT ${pType} STREQUAL "STATIC_LIBRARY")
                INSTALL(TARGETS ${T}
                    DESTINATION ${PV_INSTALL_PLUGIN_DIR}
                    COMPONENT Runtime)                                    
            ENDIF(NOT ${pType} STREQUAL "STATIC_LIBRARY")
        ENDFOREACH(T)
    ELSE(NOT BUILD_SHARED_LIBS)
        INSTALL(TARGETS ${ARGN}
          DESTINATION ${PV_INSTALL_PLUGIN_DIR}
          COMPONENT Runtime)          
    ENDIF( NOT BUILD_SHARED_LIBS)
ENDMACRO(VISIT_INSTALL_TARGETS)