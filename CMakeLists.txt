cmake_minimum_required( VERSION 2.8.7 )

set( EXTENSION_NAME PerkTutor )

set( EXTENSION_HOMEPAGE "https://github.com/PerkTutor/PerkTutor/wiki" )
set( EXTENSION_CATEGORY "IGT" )
set( EXTENSION_CONTRIBUTORS "Tamas Ungi, Matthew S. Holden (Queen's University)" )
set( EXTENSION_DESCRIPTION "This extension holds modules for training of needle interventions and analysis of operator performance." )
set( EXTENSION_ICONURL "http://www.slicer.org/slicerWiki/images/2/21/PerkTutorLogo.png" )
set( EXTENSION_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/images/2/28/PerkTutorScreenshot.png" )
set( EXTENSION_DEPENDS "NA") # Specified as a space separated list or 'NA' if any
set( EXTENSION_BUILD_SUBDIRECTORY inner-build )

find_package( Slicer REQUIRED )
include( ${Slicer_USE_FILE} )

# Run superbuild in the first round of configuration.
option(${EXTENSION_NAME}_SUPERBUILD "Build ${EXTENSION_NAME} and the projects it depends on via SuperBuild.cmake." ON)
mark_as_advanced(${EXTENSION_NAME}_SUPERBUILD)

if(${EXTENSION_NAME}_SUPERBUILD)
  include("${CMAKE_CURRENT_SOURCE_DIR}/SuperBuild.cmake")
  return()
endif()

set( MODULE_NAME ${EXTENSION_NAME} )
set( MODULE_TITLE ${MODULE_NAME} )
string( TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER )

add_subdirectory( ${PerkEvaluator_SOURCE_DIR} ${PerkEvaluator_BINARY_DIR} )
add_subdirectory( ${TargetFiducials_SOURCE_DIR} ${TargetFiducials_BINARY_DIR} )
add_subdirectory( ${TransformRecorder_SOURCE_DIR} ${TransformRecorder_BINARY_DIR} )
add_subdirectory( ${WorkflowSegmentation_SOURCE_DIR} ${WorkflowSegmentation_BINARY_DIR} )

set(CPACK_INSTALL_CMAKE_PROJECTS "${CPACK_INSTALL_CMAKE_PROJECTS};${CMAKE_BINARY_DIR};${EXTENSION_NAME};ALL;/")

include(${Slicer_EXTENSION_CPACK})
