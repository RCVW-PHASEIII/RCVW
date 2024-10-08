CMAKE_MINIMUM_REQUIRED (VERSION 3.18)

PROJECT ( message-asntype C )

SET (TMXLIB tmx${PROJECT_NAME})
SET (TMXTEST test-${TMXLIB})

SET (ASN_INCLUDE_DIR "tmx/message/codec/asn")

IF (NOT TMX_ASN_DEBUG)
	SET (TMX_ASN_DEBUG FALSE)
ENDIF ()

FILE (GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c*")
ADD_LIBRARY (${TMXLIB} OBJECT ${SOURCES})

# Use the older C standard because ASN_DEBUG is broken in C99
SET_PROPERTY (TARGET ${TMXLIB} PROPERTY C_STANDARD 90)

TARGET_INCLUDE_DIRECTORIES (${TMXLIB} SYSTEM PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/${ASN_INCLUDE_DIR}>
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src
		$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src/${ASN_INCLUDE_DIR}>)
TARGET_LINK_LIBRARIES (${TMXLIB} PRIVATE tmxmessage-platform tmxmessage-types tmxmessage-codec)

INSTALL (DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/src/
		DESTINATION include
		COMPONENT tmx-message
		FILES_MATCHING PATTERN "*.h*")
