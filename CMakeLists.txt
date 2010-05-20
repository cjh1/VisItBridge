cmake_minimum_required(VERSION 2.6)

project(VisItBridgePlugin)

# -----------------------------------------------------------------------------
# Include CTest for testing
# -----------------------------------------------------------------------------
include(CTest)
include(VisItCommon.cmake)

option(VISIT_DBIO_ONLY "Build with only Database IO libs" ON)
ADD_SUBDIRECTORY(common)
ADD_SUBDIRECTORY(visit_vtk)
ADD_SUBDIRECTORY(avt)
ADD_SUBDIRECTORY(VisItReader)

