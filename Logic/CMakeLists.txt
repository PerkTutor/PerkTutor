project(vtkSlicer${MODULE_NAME}ModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

set(module_logic_name "${PROJECT_NAME}")

# The header '${module_logic_name}Export.h' will be automatically configured.
set(module_logic_export_directive "VTK_SLICER_TRANSFORMRECORDER_MODULE_LOGIC_EXPORT")


# debug
# MESSAGE( STATUS "***LOGIC OpenIGTLinkIF_SOURCE_DIR: ${OpenIGTLinkIF_SOURCE_DIR}/MRML" )


# Additional directories to include
set(module_logic_include_directories
  ${PerkTutorCommon_SOURCE_DIR}
  ${PerkTutorCommon_BINARY_DIR}
  )

# Source files
set(module_logic_SRCS
  vtkSlicerTransformRecorderLogic.cxx
  vtkSlicerTransformRecorderLogic.h
  )

# Additional Target libraries
set(module_logic_target_libraries
  ${ITK_LIBRARIES}
  ${MRML_LIBRARIES}
  SlicerBaseLogic
  PerkTutorCommon
  vtkSlicerTransformRecorderModuleMRML
  )


SlicerMacroBuildModuleLogic(
  NAME ${module_logic_name}
  EXPORT_DIRECTIVE ${module_logic_export_directive}
  INCLUDE_DIRECTORIES ${module_logic_include_directories}
  SRCS ${module_logic_SRCS}
  TARGET_LIBRARIES ${module_logic_target_libraries}
  )
