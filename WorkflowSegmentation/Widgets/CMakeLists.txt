project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${qSlicerTransformRecorderModuleWidgets_SOURCE_DIR}
  ${qSlicerTransformRecorderModuleWidgets_BINARY_DIR}
  ${vtkSlicerTransformRecorderModuleLogic_SOURCE_DIR}
  ${vtkSlicerTransformRecorderModuleLogic_BINARY_DIR}
  ${Sequences_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  qSlicerWorkflowSegmentationRecorderControlsWidget.cxx
  qSlicerWorkflowSegmentationRecorderControlsWidget.h
  qSlicerWorkflowToolSummaryWidget.cxx
  qSlicerWorkflowToolSummaryWidget.h
  qSlicerWorkflowToolWidget.cxx
  qSlicerWorkflowToolWidget.h
  qSlicerWorkflowGuideDisplayWidget.cxx
  qSlicerWorkflowGuideDisplayWidget.h
  )
  
set(${KIT}_MOC_SRCS
  qSlicerWorkflowSegmentationRecorderControlsWidget.h
  qSlicerWorkflowToolSummaryWidget.h
  qSlicerWorkflowToolWidget.h
  qSlicerWorkflowGuideDisplayWidget.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qSlicerWorkflowToolSummaryWidget.ui
  Resources/UI/qSlicerWorkflowToolWidget.ui
  Resources/UI/qSlicerWorkflowGuideDisplayWidget.ui
  )
  
set(${KIT}_RESOURCES
  Resources/UI/qSlicerWorkflowToolSummaryWidget.ui
  Resources/UI/qSlicerWorkflowToolWidget.ui
  Resources/UI/qSlicerWorkflowGuideDisplayWidget.ui
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicerTransformRecorderModuleLogic 
  qSlicerTransformRecorderModuleWidgets
  ${Sequences_LIBS}
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
