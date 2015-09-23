project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerTransformRecorderModuleLogic_SOURCE_DIR}
  ${vtkSlicerTransformRecorderModuleLogic_BINARY_DIR}
  ${vtkSlicerTransformRecorderModuleMRML_SOURCE_DIR}
  ${vtkSlicerTransformRecorderModuleMRML_BINARY_DIR}
   )

# --------------------------------------------------------------------------
# Sources

set(${KIT}_SRCS
  vtkLabelVector.cxx
  vtkLabelVector.h
  vtkLabelRecord.cxx
  vtkLabelRecord.h
  vtkMarkovVector.cxx
  vtkMarkovVector.h
  
  vtkWorkflowTask.cxx
  vtkWorkflowTask.h
  
  vtkMarkovModel.cxx
  vtkMarkovModel.h
  vtkMarkovModelRT.cxx
  vtkMarkovModelRT.h
  
  vtkWorkflowLogRecordBuffer.cxx
  vtkWorkflowLogRecordBuffer.h
  vtkWorkflowLogRecordBufferRT.cxx
  vtkWorkflowLogRecordBufferRT.h
  
  vtkMRMLWorkflowProcedureNode.cxx
  vtkMRMLWorkflowProcedureNode.h
  vtkMRMLWorkflowProcedureStorageNode.cxx
  vtkMRMLWorkflowProcedureStorageNode.h
  vtkMRMLWorkflowInputNode.cxx
  vtkMRMLWorkflowInputNode.h
  vtkMRMLWorkflowInputStorageNode.cxx
  vtkMRMLWorkflowInputStorageNode.h
  vtkMRMLWorkflowTrainingNode.cxx
  vtkMRMLWorkflowTrainingNode.h
  vtkMRMLWorkflowTrainingStorageNode.cxx
  vtkMRMLWorkflowTrainingStorageNode.h
  vtkMRMLWorkflowToolNode.cxx
  vtkMRMLWorkflowToolNode.h
  
  vtkMRMLWorkflowSegmentationNode.cxx
  vtkMRMLWorkflowSegmentationNode.h
  )

SET (${KIT}_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} CACHE INTERNAL "" FORCE)

# --------------------------------------------------------------------------
# Build the library

set(${KIT}_TARGET_LIBRARIES
  ${ITK_LIBRARIES}
  ${MRML_LIBRARIES}
  SlicerBaseLogic
  vtkSlicerTransformRecorderModuleMRML
  )

SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )