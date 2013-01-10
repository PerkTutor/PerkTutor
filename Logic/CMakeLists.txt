project(vtkSlicer${MODULE_NAME}ModuleLogic)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_LOGIC_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
	${Slicer_Base_INCLUDE_DIRS}
  )

set(${KIT}_SRCS
  vtkSlicer${MODULE_NAME}Logic.cxx
  vtkSlicer${MODULE_NAME}Logic.h
  vtkRecordLog.cxx
  vtkRecordLog.h
  vtkProcedure.cxx
  vtkProcedure.h
  vtkTrackingRecord.cxx
  vtkTrackingRecord.h
  vtkMarkovModel.cxx
  vtkMarkovModel.h
  vtkMarkovModelRT.cxx
  vtkMarkovModelRT.h
  vtkRecordLogRT.cxx
  vtkRecordLogRT.h
  vtkWorkflowAlgorithm.cxx
  vtkWorkflowAlgorithm.h
  RecordType.h
  )

set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  ${MRML_LIBRARIES}
  SlicerBaseLogic 
  vtkSlicerWorkflowSegmentationModuleMRML
  )

#-----------------------------------------------------------------------------
SlicerMacroBuildModuleLogic(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )