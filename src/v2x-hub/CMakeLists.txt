#
# V2X Hub Plugins
# Makefile configuration script
#
# Author:
#    Gregory M. Baumgardner
# Version:
#    1.0 : Initial version
#    2.0 : Release 4.0.0 updates
#

CMAKE_MINIMUM_REQUIRED (VERSION 3.18)

PROJECT (v2x-hub LANGUAGES NONE VERSION 4.0.0 DESCRIPTION "V2X Hub Reference Implementation")

IF (COMMAND SETUP_TMX_PROJECT)
    SETUP_TMX_PROJECT ()
ENDIF ()

ENABLE_LANGUAGE ( C )
ENABLE_LANGUAGE ( CXX )

INCLUDE (CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG ("-fPIC" CMAKE_C_FLAG_FPIC_SUPPORTED)

IF (NOT TMX_BIN_DIR)
    SET (TMX_BIN_DIR "${CMAKE_BINARY_DIR}/bin")
ENDIF (NOT TMX_BIN_DIR)

IF (NOT TMX_LIB_DIR)
    SET (TMX_LIB_DIR "${CMAKE_BINARY_DIR}/lib")
ENDIF (NOT TMX_LIB_DIR)

IF (TMX_CROSSCOMPILING)
    MESSAGE (STATUS "Cross-compiling for ${CMAKE_SYSTEM} on ${CMAKE_SYSTEM_PROCESSOR}")
ENDIF (TMX_CROSSCOMPILING)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/ICD/include)

FOREACH (Plugin "MapPlugin" "RtcmPlugin" "RSUImmediateForwardPlugin" "MessageReceiverPlugin" )
    IF (NOT SKIP_${Plugin})
        ADD_SUBDIRECTORY (${Plugin})
    ENDIF ()
ENDFOREACH ()
