#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.7)

#-----------------------------------------------------------------------------
set(MODULE_NAME "TargetFiducials") # Do not use 'project()'
set(MODULE_TITLE "Target Fiducials")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
add_subdirectory(Logic)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )

set(MODULE_SRCS
  qSlicerTargetFiducialsModule.cxx
  qSlicerTargetFiducialsModule.h
  qSlicerTargetFiducialsModuleWidget.cxx
  qSlicerTargetFiducialsModuleWidget.h
  )

set(MODULE_MOC_SRCS
  qSlicerTargetFiducialsModule.h
  qSlicerTargetFiducialsModuleWidget.h
  )

set(MODULE_UI_SRCS
  Resources/UI/qSlicerTargetFiducialsModule.ui
  )

set(MODULE_TARGET_LIBRARIES
  vtkSlicerTargetFiducialsModuleLogic
  vtkSlicerAnnotationsModuleMRML
  )

set(MODULE_RESOURCES
  Resources/qSlicerTargetFiducialsModule.qrc
  )

#-----------------------------------------------------------------------------
slicerMacroBuildQtModule(
  NAME ${MODULE_NAME}
  TITLE ${MODULE_TITLE}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  SRCS ${MODULE_SRCS}
  MOC_SRCS ${MODULE_MOC_SRCS}
  UI_SRCS ${MODULE_UI_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  RESOURCES ${MODULE_RESOURCES}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
