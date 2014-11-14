set(the_description "The UbiTrack FaroArm Networking Component")
ut_add_component(faro DEPS utcore utdataflow )

IF(WIN32 AND DEFINED EXTERNAL_LIBRARIES_DIR)
	set(FAROSDK_ROOT "${EXTERNAL_LIBRARIES_DIR}/farosdk")
ENDIF(WIN32 AND DEFINED EXTERNAL_LIBRARIES_DIR)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

FIND_PACKAGE(FAROSDK)

IF(FAROSDK_FOUND)
  ut_component_include_directories("src/FaroArm" ${FAROSDK_INCLUDE_DIR} ${UBITRACK_CORE_DEPS_INCLUDE_DIR})
  ut_glob_component_sources(HEADERS "src/FaroArm/*.h" SOURCES "src/FaroArm/*.cpp")
  ut_create_single_component(${FAROSDK_LIBRARY})
ENDIF(FAROSDK_FOUND)

ut_install_utql_patterns()
