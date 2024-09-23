#
# Internal development make configuration file
#
# This file is a basic CMake build file that enables certain capabilities that 
# are intended to only be available for TMX development, including component packaging.
#
# Not to be distribted!
#
# Author:
#    Gregory M. Baumgardner
#
# Version:
#    1.0 : Initial version
#    2.0 : Updates for repackaging
#

CMAKE_MINIMUM_REQUIRED (VERSION 3.18)

PROJECT ( "Transportation Message eXchange" NONE )

# Determine build type
IF (NOT CMAKE_BUILD_TYPE)
	IF ("${CMAKE_CURRENT_BINARY_DIR}" MATCHES ".*Debug")
		SET (CMAKE_BUILD_TYPE Debug CACHE STRING "Build type")
	ELSEIF ("${CMAKE_CURRENT_BINARY_DIR}" MATCHES ".*Release")
		SET (CMAKE_BUILD_TYPE Release CACHE STRING "Build type")
	ENDIF ()
ENDIF (NOT CMAKE_BUILD_TYPE)

IF (NOT TMX_PROJECT)
    SET (TMX_PROJECT tmx)
ENDIF ()

MACRO (SETUP_TMX_PROJECT)
    # Global values
    SET (TMX_APPNAME ${PROJECT_NAME} PARENT_SCOPE)
    SET (TMX_VERSION ${PROJECT_VERSION} PARENT_SCOPE)
    SET (TMX_VERSION_MAJOR ${PROJECT_VERSION_MAJOR} PARENT_SCOPE)
    SET (TMX_VERSION_MINOR ${PROJECT_VERSION_MINOR} PARENT_SCOPE)
    SET (TMX_VERSION_PATCH ${PROJECT_VERSION_PATCH} PARENT_SCOPE)
    SET (TMX_VERSION_TWEAK ${PROJECT_VERSION_TWEAK} PARENT_SCOPE)

    MESSAGE (STATUS "Building ${TMX_APPNAME} release ${TMX_VERSION} from ${CMAKE_CURRENT_SOURCE_DIR} to ${CMAKE_CURRENT_BINARY_DIR}")
ENDMACRO()

GET_FILENAME_COMPONENT (TMX_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../src/${TMX_PROJECT}" ABSOLUTE)
ADD_SUBDIRECTORY(${TMX_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${TMX_PROJECT})

# General package information
SET (CPACK_PROJECT_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/CPackCoreConfig.cmake")

INCLUDE (${CPACK_PROJECT_CONFIG_FILE})

FIND_PROGRAM (Docker docker)

ADD_CUSTOM_TARGET (package-all
                    "${CMAKE_COMMAND}" --build "${CMAKE_CURRENT_BINARY_DIR}" --target package
                    COMMENT "Packaging ${TMX_APPNAME}")

IF (Docker)
    GET_FILENAME_COMPONENT (BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} NAME)
    STRING (REPLACE "-" ";" BUILD_DIR_LIST ${BUILD_DIR})
    LIST (POP_FRONT BUILD_DIR_LIST TMX_ARCH)
    LIST (POP_FRONT BUILD_DIR_LIST TMX_CONFIG)

    # If there is a Docker deployment available, then make targets for each
    FILE (GLOB TMX_CONTAINERS ${CMAKE_CURRENT_SOURCE_DIR}/Docker/deploy/${TMX_APPNAME}-*/docker-compose.yaml)
    FOREACH (TMX_DEPLOYMENT ${TMX_CONTAINERS})
        GET_FILENAME_COMPONENT(TMX_DEPLOYMENT_DIR ${TMX_DEPLOYMENT} DIRECTORY)

        # Create a list of the services
        EXECUTE_PROCESS(COMMAND ${Docker} compose --project-directory "${TMX_DEPLOYMENT_DIR}" config --services
                        OUTPUT_VARIABLE TMX_SERVICES_OUT)
        STRING (REPLACE "\n" ";" TMX_SERVICES "${TMX_SERVICES_OUT}")

        # Loop through each service
        FOREACH (TMX_SERVICE_NAME ${TMX_SERVICES})
            ADD_CUSTOM_TARGET (docker-image-${TMX_SERVICE_NAME}
                                ${Docker} compose --project-directory "${TMX_DEPLOYMENT_DIR}" build
                                    --build-arg TMX_ARCH="${TMX_ARCH}"
                                    --build-arg TMX_CONFIG="${TMX_CONFIG}"
                                    --build-arg TMX_VERSION="${TMX_VERSION}"
                                    ${TMX_SERVICE_NAME}
                                COMMENT "Creating ${TMX_SERVICE_NAME} Docker environment")

            FILE (GLOB TMX_DEPENDS_FILES "${TMX_DEPLOYMENT_DIR}/${TMX_SERVICE_NAME}.depends.*")
            FOREACH (TMX_DEPENDS_FILE ${TMX_DEPENDS_FILES})
                GET_FILENAME_COMPONENT (DEPENDS "${TMX_DEPENDS_FILE}" LAST_EXT)
                STRING (SUBSTRING ${DEPENDS} 1 -1 DEPENDS)
                ADD_DEPENDENCIES (docker-image-${TMX_SERVICE_NAME} ${DEPENDS})
                MESSAGE(STATUS "Added dependency ${DEPENDS} to docker-image-${TMX_SERVICE_NAME}")
            ENDFOREACH (TMX_DEPENDS_FILE ${TMX_DEPENDS_FILES})

            ADD_CUSTOM_TARGET (deploy-${TMX_SERVICE_NAME}
                                ${Docker} compose --project-directory "${TMX_DEPLOYMENT_DIR}" up -d ${TMX_SERVICE_NAME}
                                DEPENDS docker-image-${TMX_SERVICE_NAME}
                                COMMENT "Launching ${TMX_SERVICE_NAME} Docker environment")
        ENDFOREACH (TMX_SERVICE_NAME ${TMX_SERVICES})
    ENDFOREACH (TMX_DEPLOYMENT ${TMX_CONTAINERS})


ENDIF (Docker)