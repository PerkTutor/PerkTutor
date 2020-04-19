project(qSlicer${MODULE_NAME}ModuleWidgets)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "Q_SLICER_MODULE_${MODULE_NAME_UPPER}_WIDGETS_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerSequencesModuleMRML_INCLUDE_DIRS}  
  ${vtkSlicerSequencesModuleLogic_INCLUDE_DIRS}  
  )

set(${KIT}_SRCS
  qSlicerTrackedSequenceMessagesWidget.h
  qSlicerTrackedSequenceMessagesWidget.cxx
  qSlicerTrackedSequenceRecorderControlsWidget.h
  qSlicerTrackedSequenceRecorderControlsWidget.cxx
  qSlicerTrackedSequenceBrowserWidget.h
  qSlicerTrackedSequenceBrowserWidget.cxx
  )

set(${KIT}_MOC_SRCS
  qSlicerTrackedSequenceMessagesWidget.h
  qSlicerTrackedSequenceRecorderControlsWidget.h
  qSlicerTrackedSequenceBrowserWidget.h
  )

set(${KIT}_UI_SRCS
  Resources/UI/qSlicerTrackedSequenceMessagesWidget.ui
  Resources/UI/qSlicerTrackedSequenceRecorderControlsWidget.ui
  Resources/UI/qSlicerTrackedSequenceBrowserWidget.ui
  )

set(${KIT}_RESOURCES
  )

set(${KIT}_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicerSequencesModuleMRML
  vtkSlicerSequencesModuleLogic
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


#-----------------------------------------------------------------------------
