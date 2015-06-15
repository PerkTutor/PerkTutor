project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${PerkTutorCommon_SOURCE_DIR}
  ${PerkTutorCommon_BINARY_DIR}
  )

set(${KIT}_SRCS
  qSlicerWorkflowSegmentationRecorderControlsWidget.cxx
  qSlicerWorkflowSegmentationRecorderControlsWidget.h
  qSlicerWorkflowToolSummaryWidget.cxx
  qSlicerWorkflowToolSummaryWidget.h
  qSlicerWorkflowToolWidget.cxx
  qSlicerWorkflowToolWidget.h
  )
  
set(${KIT}_MOC_SRCS
  qSlicerWorkflowSegmentationRecorderControlsWidget.h
  qSlicerWorkflowToolSummaryWidget.h
  qSlicerWorkflowToolWidget.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qSlicerWorkflowToolSummaryWidget.ui
  Resources/UI/qSlicerWorkflowToolWidget.ui
  )
  
set(${KIT}_RESOURCES
  Resources/UI/qSlicerWorkflowToolSummaryWidget.ui
  Resources/UI/qSlicerWorkflowToolWidget.ui
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  PerkTutorCommon
  qSlicerTransformRecorderModuleWidgets
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleWidgets(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  MOC_SRCS ${${KIT}_MOC_SRCS}
  UI_SRCS ${${KIT}_UI_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  RESOURCES ${${KIT}_RESOURCES}
  WRAP_PYTHONQT
  )
