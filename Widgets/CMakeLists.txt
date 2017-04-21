project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  )

set(${KIT}_SRCS
  qSlicerAssessmentTableWidget.cxx
  qSlicerAssessmentTableWidget.h
  qSlicerMetricWeightWidget.cxx
  qSlicerMetricWeightWidget.h
  )

set(${KIT}_MOC_SRCS
  qSlicerAssessmentTableWidget.h
  qSlicerMetricWeightWidget.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qSlicerAssessmentTableWidget.ui
  Resources/UI/qSlicerMetricWeightWidget.ui
  )

set(${KIT}_RESOURCES
  )

#-----------------------------------------------------------------------------
set(${KIT}_TARGET_LIBRARIES
  MRMLCore
  MRMLLogic
  ${QT_LIBRARIES}
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

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  #add_subdirectory(Testing)
endif()
