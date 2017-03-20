project(vtkSlicer${MODULE_NAME}ModuleLogic)

#
# See CMake/SlicerMacroBuildModuleLogic.cmake for details
#

set(KIT "${PROJECT_NAME}")

# The header '${module_logic_name}Export.h' will be automatically configured.
set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")


# Additional directories to include
set(${KIT}_INCLUDE_DIRECTORIES
  ${Sequences_INCLUDE_DIRS}
  )

# Source files
set(${KIT}_SRCS
  vtkSlicerTransformRecorderLogic.cxx
  vtkSlicerTransformRecorderLogic.h
  )

# Additional Target libraries
set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  ${MRML_LIBRARIES}
  SlicerBaseLogic
  ${Sequences_LIBS}
  )


#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )
