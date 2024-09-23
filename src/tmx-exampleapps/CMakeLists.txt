#
# Example Plugins
# Makefile configuration script
#
# Author:
#    Gregory M. Baumgardner
# Version:
#    1.0 : Initial version
#

CMAKE_MINIMUM_REQUIRED (VERSION 3.5)

# Global values
SET (TMX_APPNAME tmx-exampleapps CACHE INTERNAL "Application name")
SET (TMX_APPDESCR "TMX Example Applications" CACHE INTERNAL "Application description")

PROJECT (${TMX_APPNAME} VERSION 4.0.0)

ENABLE_LANGUAGE ( C )
ENABLE_LANGUAGE ( CXX )

IF (TMX_CROSSCOMPILING)
    MESSAGE (STATUS "Cross-compiling for ${CMAKE_SYSTEM} on ${CMAKE_SYSTEM_PROCESSOR}")
ENDIF (TMX_CROSSCOMPILING)

STRING (REPLACE ${CMAKE_SOURCE_DIR}/ "" TMX_BUILD_SUBDIR "${CMAKE_BINARY_DIR}")

ADD_SUBDIRECTORY(${CMAKE_SOURCE_DIR}/../tmx ${CMAKE_SOURCE_DIR}/../tmx/${TMX_BUILD_SUBDIR})
ADD_SUBDIRECTORY(EmptyPlugin)
ADD_SUBDIRECTORY(ExamplePlugin)