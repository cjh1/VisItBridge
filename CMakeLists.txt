cmake_minimum_required(VERSION 2.6)

project(VisItBridgePlugin)

# -----------------------------------------------------------------------------
# Include CTest for testing
# -----------------------------------------------------------------------------
include(CTest)

option(VISIT_DBIO_ONLY "Build with only Database IO libs" ON)
ADD_SUBDIRECTORY(avt)
ADD_SUBDIRECTORY(plugin)

