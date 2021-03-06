#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8.7)

#-----------------------------------------------------------------------------
# Module name (usually identique to the directory name)
set(MODULE_NAME "TransformRecorder")
set(MODULE_TITLE "Transform Recorder")

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
# Build module sub libraries
add_subdirectory(Logic)
add_subdirectory(Widgets)

set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${vtkSlicerSequencesModuleMRML_INCLUDE_DIRS}  
  ${vtkSlicerSequencesModuleLogic_INCLUDE_DIRS}  
  )

# Source files
set(MODULE_SRCS
  qSlicer${MODULE_NAME}Module.cxx
  qSlicer${MODULE_NAME}Module.h
  qSlicer${MODULE_NAME}ModuleWidget.cxx
  qSlicer${MODULE_NAME}ModuleWidget.h
  qSlicerTrackedSequenceBrowserReader.cxx
  qSlicerTrackedSequenceBrowserReader.h
  qSlicerTrackedSequenceBrowserWriter.cxx
  qSlicerTrackedSequenceBrowserWriter.h
  qSlicerTrackedSequenceBrowserReaderOptionsWidget.cxx
  qSlicerTrackedSequenceBrowserReaderOptionsWidget.h
  )

# Headers that should run through moc
set(MODULE_MOC_SRCS
  qSlicer${MODULE_NAME}Module.h
  qSlicer${MODULE_NAME}ModuleWidget.h
  qSlicerTrackedSequenceBrowserReader.h
  qSlicerTrackedSequenceBrowserWriter.h
  qSlicerTrackedSequenceBrowserReaderOptionsWidget.h
  )

# UI files
set(MODULE_UI_SRCS
  Resources/UI/qSlicer${MODULE_NAME}Module.ui
  Resources/UI/qSlicerTrackedSequenceBrowserReaderOptionsWidget.ui
  )
 
# Additional Target libraries
set(MODULE_TARGET_LIBRARIES
  qSlicer${MODULE_NAME}ModuleWidgets
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicerSequencesModuleMRML
  vtkSlicerSequencesModuleLogic
  )

# Resources
set(MODULE_RESOURCES
  Resources/qSlicer${MODULE_NAME}Module.qrc
  )

#-----------------------------------------------------------------------------
slicerMacroBuildLoadableModule(
  NAME ${MODULE_NAME}
  TITLE ${MODULE_TITLE}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  SRCS ${MODULE_SRCS}
  MOC_SRCS ${MODULE_MOC_SRCS}
  UI_SRCS ${MODULE_UI_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  RESOURCES ${MODULE_RESOURCES}
  WITH_GENERIC_TESTS
  )

 
#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
