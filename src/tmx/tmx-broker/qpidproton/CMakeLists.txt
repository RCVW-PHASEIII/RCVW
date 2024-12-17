CMAKE_MINIMUM_REQUIRED (VERSION 3.18)

PROJECT ( broker-qpidproton C CXX )

SET (TMXLIB tmx${PROJECT_NAME})
SET (TMXTEST test-${TMXLIB})

FIND_PACKAGE (Proton QUIET)

IF (Proton_FOUND)
    MESSAGE (STATUS "Including support for Qpid-Proton broker")

    FILE (GLOB_RECURSE SOURCES "src/*.c*")

    ADD_LIBRARY (${TMXLIB} OBJECT ${SOURCES})
    TARGET_INCLUDE_DIRECTORIES (${TMXLIB} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
    TARGET_LINK_LIBRARIES (${TMXLIB} PUBLIC tmxbroker-api qpid-proton-cpp
                                            ${Proton_LIBRARIES} ${Proton_Core_LIBRARIES} ${Proton_Proactor_LIBRARIES})
    TARGET_LINK_LIBRARIES (tmx-broker INTERFACE qpid-proton-cpp
                                                ${Proton_LIBRARIES} ${Proton_Core_LIBRARIES} ${Proton_Proactor_LIBRARIES})
ENDIF ()