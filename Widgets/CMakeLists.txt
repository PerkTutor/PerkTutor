project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
  qSlicerPerkEvaluatorMessagesWidget.cxx
  qSlicerPerkEvaluatorMessagesWidget.h
  qSlicerMetricsTableWidget.cxx
  qSlicerMetricsTableWidget.h
  qSlicerPerkEvaluatorRecorderControlsWidget.cxx
  qSlicerPerkEvaluatorRecorderControlsWidget.h
  qSlicerPerkEvaluatorRolesWidget.cxx
  qSlicerPerkEvaluatorRolesWidget.h
  qSlicerPerkEvaluatorTransformRolesWidget.cxx
  qSlicerPerkEvaluatorTransformRolesWidget.h
  qSlicerPerkEvaluatorAnatomyRolesWidget.cxx
  qSlicerPerkEvaluatorAnatomyRolesWidget.h
  )
  
set(${KIT}_MOC_SRCS
  qSlicerPerkEvaluatorRolesWidget.h
  qSlicerPerkEvaluatorMessagesWidget.h
  qSlicerMetricsTableWidget.h
  qSlicerPerkEvaluatorRecorderControlsWidget.h
  qSlicerPerkEvaluatorTransformRolesWidget.h
  qSlicerPerkEvaluatorAnatomyRolesWidget.h
  )
  
set(${KIT}_UI_SRCS
  Resources/UI/qSlicerPerkEvaluatorRolesWidget.ui
  Resources/UI/qSlicerMetricsTableWidget.ui
  )

set(${KIT}_RESOURCES
  Resources/UI/qSlicerPerkEvaluatorRolesWidget.ui
  Resources/UI/qSlicerMetricsTableWidget.ui
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  qSlicerTransformRecorderModuleWidgets
  vtkSlicerTransformRecorderModuleLogic
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

