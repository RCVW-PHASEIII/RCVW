#
# Internal development core packaging configuration file
# 
# This file contains the packaging information for the TMX core components.
# There are variables set in this file that pertain to the packaging, but if there 
# are specialized configurations for the different package generators, then those 
# also belong inside this file.
#
# Not to be distributed!
#
# Author:
#    Gregory M. Baumgardner
#
# Version:
#    1.0 : Initial version
#

# Set up default variables from included file
IF (NOT CPACK_GENERATOR)
    MESSAGE (STATUS "Configuring packaging for ${TMX_APPNAME} release ${TMX_VERSION}")
    
    # Common Packaging configuration.  The include(CPack) must be done after this include
    SET (CPACK_GENERATOR "ZIP;TGZ")                        # Always Build a ZIP and GZip Tarball distribution
    IF (CMAKE_SYSTEM_NAME MATCHES "^[lL]inux")
        SET (CPACK_GENERATOR "${CPACK_GENERATOR};DEB")     # Build .deb package distribution also for Linux
    ENDIF ()
    
    # Common variables
    SET (CPACK_SET_DESTDIR ON)
    SET (CPACK_PACKAGE_NAME ${TMX_APPNAME})
    SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY ${TMX_APPDESCR})
    SET (CPACK_PACKAGE_VERSION_MAJOR ${TMX_VERSION_MAJOR})
    SET (CPACK_PACKAGE_VERSION_MINOR ${TMX_VERSION_MINOR})
    SET (CPACK_PACKAGE_VERSION_PATCH ${TMX_VERSION_PATCH})

    SET (CPACK_INCLUDE_TOPLEVEL_DIRECTORY "OFF")           # Do not include a sub-directory for the project
    SET (CPACK_PACKAGE_VENDOR "battelle.org")              # A Battelle product
    SET (CPACK_PACKAGE_CONTACT "baumgardner@battelle.org") # Build Administrator
    
#    SET (CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
    
    # Make sure component installs are turned on
    SET (CPACK_ARCHIVE_COMPONENT_INSTALL ON)
    SET (CPACK_DEB_COMPONENT_INSTALL ON)
    
    IF (DEFINED ENV{CPACK_VERSION_SUFFIX})
        SET (CPACK_PACKAGE_VERSION_PATCH "${CPACK_PACKAGE_VERSION_PATCH}-$ENV{CPACK_VERSION_SUFFIX}")
    ENDIF ()
    IF (DEFINED CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
        SET (CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})
    ELSE ()
        SET (CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR})
    ENDIF ()
        
    # Specific package information     
    # ASN.1 Libraries
    SET (CPACK_DEBIAN_LIBASN_J2735_R41_PACKAGE_NAME libasn_j2735_r41)
    SET (CPACK_COMPONENT_LIBASN_J2735_R41_DISPLAY_NAME "SAE J2735 2015 C library and headers")
    SET (CPACK_COMPONENT_LIBASN_J2735_R41_DESCRIPTION "${CPACK_COMPONENT_LIBASN_J2735_R41_DISPLAY_NAME}")     
    SET (CPACK_DEBIAN_LIBASN_J2735_R63_PACKAGE_NAME libasn_j2735_r63)
    SET (CPACK_COMPONENT_LIBASN_J2735_R63_DISPLAY_NAME "SAE J2735 2016 C library and headers")
    SET (CPACK_COMPONENT_LIBASN_J2735_R63_DESCRIPTION "${CPACK_DEBIAN_LIBASN_J2735_R63_PACKAGE_NAME}")        
    
    # TMX API Library
    SET (CPACK_DEBIAN_LIBTMXAPI_PACKAGE_NAME lib${CPACK_PACKAGE_NAME}api)
    IF (CPACK_PACKAGE_VERSION VERSION_GREATER 3 OR CPACK_PACKAGE_VERSION VERSION_EQUAL 3)
        SET (CPACK_DEBIAN_LIBTMXAPI_PACKAGE_CONFLICTS "lib${CPACK_PACKAGE_NAME}api (< 3.0.0)")
    ELSE ()
        SET (CPACK_DEBIAN_LIBTMXAPI_PACKAGE_CONFLICTS "lib${CPACK_PACKAGE_NAME}api (>= 3.0.0)")
    ENDIF ()
    
    SET (CPACK_COMPONENT_LIBTMXAPI_DISPLAY_NAME "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} C API library and headers")
    SET (CPACK_COMPONENT_LIBTMXAPI_DESCRIPTION "${CPACK_COMPONENT_LIBTMXAPI_DISPLAY_NAME}")
    IF (NOT Boost_VERSION)
    	SET (Boost_VERSION 0.0)
    ENDIF ()
    SET (CPACK_DEBIAN_LIBTMXAPI_PACKAGE_DEPENDS "libstdc++6" 
                                                "${CPACK_DEBIAN_LIBASN_J2735_R63_PACKAGE_NAME}"
                                                "libboost-system${Boost_VERSION}-dev" 
                                                "libboost-thread${Boost_VERSION}-dev"
                                                "libboost-atomic${Boost_VERSION}-dev" 
                                                "libboost-chrono${Boost_VERSION}-dev"
                                                "libboost-regex${Boost_VERSION}-dev" 
                                                "libboost-log${Boost_VERSION}-dev"
                                                "libboost-filesystem${Boost_VERSION}-dev"
                                                "libboost-program-options${Boost_VERSION}-dev")
    STRING (REGEX REPLACE ";" ", " CPACK_DEBIAN_LIBTMXAPI_PACKAGE_DEPENDS "${CPACK_DEBIAN_LIBTMXAPI_PACKAGE_DEPENDS}")
    
    # TMX Messages
    SET (CPACK_DEBIAN_TMXMESSAGES_PACKAGE_NAME ${CPACK_PACKAGE_NAME}-messages)
    SET (CPACK_COMPONENT_TMXMESSAGES_DISPLAY_NAME "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} C++ message definitions")
    SET (CPACK_COMPONENT_TMXMESSAGES_DESCRIPTION "${CPACK_COMPONENT_TMXMESSAGES_DISPLAY_NAME}")

    # TMX C++ Runtime Library
    SET (CPACK_DEBIAN_LIBTMXC++_PACKAGE_NAME lib${CPACK_PACKAGE_NAME}c++)
    SET (CPACK_COMPONENT_LIBTMXC++_DISPLAY_NAME "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} C++ runtime library and headers")
    SET (CPACK_COMPONENT_LIBTMXC++_DESCRIPTION "${CPACK_COMPONENT_LIBTMXC++_DISPLAY_NAME}")
    SET (CPACK_DEBIAN_LIBTMXC++_PACKAGE_DEPENDS libuuid1 libssl1.1 ${CPACK_DEBIAN_LIBTMXAPI_PACKAGE_NAME})
    
    # TMX Utils Library
    SET (CPACK_DEBIAN_LIBTMXUTILS_PACKAGE_NAME lib${CPACK_PACKAGE_NAME}utils)
    SET (CPACK_COMPONENT_LIBTMXUTILS_DISPLAY_NAME "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} C++ plugin utility library and headers")
    SET (CPACK_COMPONENT_LIBTMXUTILS_DESCRIPTION "${CPACK_COMPONENT_LIBTMXUTILS_DISPLAY_NAME}")
    SET (CPACK_DEBIAN_LIBTMXUTILS_PACKAGE_DEPENDS ${CPACK_DEBIAN_LIBTMXC++_PACKAGE_NAME})
    
    # TMX Core
    SET (CPACK_DEBIAN_TMXCORE_PACKAGE_NAME ${CPACK_PACKAGE_NAME}core)
    SET (CPACK_COMPONENT_TMXCORE_DISPLAY_NAME "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} server")
    SET (CPACK_COMPONENT_TMXCORE_DESCRIPTION "${CPACK_COMPONENT_TMXCORE_DISPLAY_NAME}")
    SET (CPACK_DEBIAN_TMXCORE_PACKAGE_DEPENDS adduser debconf mysql-server systemd libmysqlcppconn7v5
                                              "${CPACK_DEBIAN_LIBTMXAPI_PACKAGE_NAME}")
    SET (CPACK_DEBIAN_TMXCORE_PACKAGE_CONTROL_EXTRA "${CMAKE_DEBIAN_PACKAGING_DIR}/dpkg/TmxCore/postinst"
                                                    "${CMAKE_DEBIAN_PACKAGING_DIR}/dpkg/TmxCore/prerm"
                                                    "${CMAKE_DEBIAN_PACKAGING_DIR}/dpkg/TmxCore/postrm"
                                                    "${CMAKE_DEBIAN_PACKAGING_DIR}/dpkg/TmxCore/templates")
    STRING (REGEX REPLACE ";" ", " CPACK_DEBIAN_TMXCORE_PACKAGE_DEPENDS "${CPACK_DEBIAN_TMXCORE_PACKAGE_DEPENDS}")
    
    # TMX Control
    SET (CPACK_DEBIAN_TMXCTL_PACKAGE_NAME ${CPACK_PACKAGE_NAME}ctl)
    SET (CPACK_COMPONENT_TMXCTL_DISPLAY_NAME "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} command line control")
    SET (CPACK_COMPONENT_TMXCTL_DESCRIPTION "${CPACK_COMPONENT_TMXCTL_DISPLAY_NAME}")
#    SET (CPACK_DEBIAN_TMXCTL_PACKAGE_DEPENDS ${CPACK_DEBIAN_LIBTMXC++_PACKAGE_NAME} file)
#    STRING (REGEX REPLACE ";" ", " CPACK_DEBIAN_TMXCTL_PACKAGE_DEPENDS "${CPACK_DEBIAN_TMXCTL_PACKAGE_DEPENDS}")

    # TMX Tools
    SET (CPACK_DEBIAN_TMXTOOLS_PACKAGE_NAME ${CPACK_PACKAGE_NAME}-tools)
    SET (CPACK_COMPONENT_TMXTOOLS_DISPLAY_NAME "${CPACK_PACKAGE_DESCRIPTION_SUMMARY} server tools")
    SET (CPACK_COMPONENT_TMXTOOLS_DESCRIPTION "${CPACK_COMPONENT_TMXCLI_DISPLAY_NAME}")
    SET (CPACK_DEBIAN_TMXTOOLS_PACKAGE_DEPENDS ${CPACK_DEBIAN_LIBTMXC++_PACKAGE_NAME})
    STRING (REGEX REPLACE ";" ", " CPACK_DEBIAN_LIBTMXC++_PACKAGE_DEPENDS "${CPACK_DEBIAN_LIBTMXC++_PACKAGE_DEPENDS}")

set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA 
    ${CMAKE_CURRENT_SOURCE_DIR}/debian/postinst
    ${CMAKE_CURRENT_SOURCE_DIR}/debian/prerm)

    INCLUDE (CPack)
ENDIF (NOT CPACK_GENERATOR)    
