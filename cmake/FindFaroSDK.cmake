# - Try to find FAROSDK
# Once done this will define
#
#  FAROSDK_FOUND - system has FAROSDK
#  FAROSDK_INCLUDE_DIRS - the FAROSDK include directory
#  FAROSDK_LIBRARIES - Link these to use FAROSDK
#  FAROSDK_DEFINITIONS - Compiler switches required for using FAROSDK
#
#

if (FAROSDK_LIBRARIES AND FAROSDK_INCLUDE_DIRS)
  # in cache already
  set(FAROSDK_FOUND TRUE)
else (FAROSDK_LIBRARIES AND FAROSDK_INCLUDE_DIRS)

  find_path(FAROSDK_INCLUDE_DIR
    NAMES
      faroscannerapi.h
    PATHS
      ${FAROSDK_ROOT}/include
      /usr/include
      /usr/local/include
      /opt/local/include
      /sw/include
  )

  find_library(FAROSDK_LIBRARY
    NAMES
      FaroArmUsbWrapper
    PATHS
      ${FAROSDK_ROOT}/lib
      /usr/lib
      /usr/local/lib
      /opt/local/lib
      /sw/lib
  )

  set(FAROSDK_INCLUDE_DIRS
    ${FAROSDK_INCLUDE_DIR}
  )

  if (FAROSDK_LIBRARY)
    set(FAROSDK_LIBRARIES
        ${FAROSDK_LIBRARIES}
        ${FAROSDK_LIBRARY}
    )
  endif (FAROSDK_LIBRARY)

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(FAROSDK DEFAULT_MSG FAROSDK_LIBRARIES FAROSDK_INCLUDE_DIRS)

  # show the FAROSDK_INCLUDE_DIRS and FAROSDK_LIBRARIES variables only in the advanced view
  mark_as_advanced(FAROSDK_INCLUDE_DIRS FAROSDK_LIBRARIES)

endif (FAROSDK_LIBRARIES AND FAROSDK_INCLUDE_DIRS)

