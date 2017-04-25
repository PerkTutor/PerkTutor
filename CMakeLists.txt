cmake_minimum_required( VERSION 2.8.7 )
cmake_policy( SET CMP0020 NEW )

project( PerkTutor )

# Extension meta-information
set( EXTENSION_NAME "PerkTutor" )
set( EXTENSION_HOMEPAGE "https://github.com/PerkTutor/PerkTutor/wiki" )
set( EXTENSION_CATEGORY "Training" )
set( EXTENSION_CONTRIBUTORS "Tamas Ungi, Matthew S. Holden (Queen's University)" )
set( EXTENSION_DESCRIPTION "This extension holds modules for training of needle interventions and analysis of operator performance." )
set( EXTENSION_ICONURL "http://www.slicer.org/slicerWiki/images/2/21/PerkTutorLogo.png" )
set( EXTENSION_SCREENSHOTURLS "http://www.slicer.org/slicerWiki/images/2/28/PerkTutorScreenshot.png" )
set( EXTENSION_DEPENDS "Sequences" ) # Specified as a space separated list or 'NA' if any


# Extension dependencies
find_package( Slicer REQUIRED )
include( ${Slicer_USE_FILE} )

find_package( Sequences REQUIRED )

set(Sequences_ADDITIONAL_MODULE_PATHS
  ${Sequences_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}
  ${Sequences_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/Debug
  ${Sequences_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/Release
  ${Sequences_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
  )

  
# Extension modules
add_subdirectory(TransformRecorder)
add_subdirectory(PerkEvaluator)
add_subdirectory(PythonMetricsCalculator)
add_subdirectory(WorkflowSegmentation)

OPTION(PERKTUTOR_ENABLE_EXPERIMENTAL_MODULES "Enable the building of work-in-progress, experimental modules." OFF)
if (PERKTUTOR_ENABLE_EXPERIMENTAL_MODULES)
  add_subdirectory(TargetFiducials)
  add_subdirectory(TissueModelCreator)
endif()

include(${Slicer_EXTENSION_CPACK})
