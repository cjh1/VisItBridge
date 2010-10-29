#
# Find the native CGNS includes and library
#
# CGNS_INCLUDE_DIR - where to find cgns.h, etc.
# CGNS_LIBRARIES   - List of fully qualified libraries to link against when using CGNS.
# CGNS_FOUND       - Do not attempt to use CGNS if "no" or undefined.

FIND_PATH(CGNS_INCLUDE_DIR cgnslib.h
  /usr/local/include
  /usr/include
)

FIND_LIBRARY(CGNS_LIBRARY cgns
  /usr/local/lib
  /usr/lib
)

IF(CGNS_INCLUDE_DIR)
  IF(CGNS_LIBRARY)
    SET( CGNS_LIBRARIES ${CGNS_LIBRARY} )
    SET( CGNS_FOUND "YES" )
  ENDIF(CGNS_LIBRARY)
ENDIF(CGNS_INCLUDE_DIR)

IF(CGNS_FIND_REQUIRED AND NOT CGNS_FOUND)
  message(SEND_ERROR "Unable to find the requested CGNS libraries.")
ENDIF(CGNS_FIND_REQUIRED AND NOT CGNS_FOUND)

MARK_AS_ADVANCED(
  CGNS_INCLUDE_DIR
  CGNS_LIBRARY
)