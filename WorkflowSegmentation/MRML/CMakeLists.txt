project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${vtkSlicerSequencesModuleMRML_INCLUDE_DIRS}  
  ${vtkSlicerSequenceBrowserModuleMRML_INCLUDE_DIRS}
   )

# --------------------------------------------------------------------------
# Sources

set(${KIT}_SRCS  
  vtkWorkflowTask.cxx
  vtkWorkflowTask.h
  
  vtkMarkovModel.cxx
  vtkMarkovModel.h
  vtkMarkovModelOnline.cxx
  vtkMarkovModelOnline.h
  
  vtkMRMLWorkflowSequenceNode.cxx
  vtkMRMLWorkflowSequenceNode.h
  vtkMRMLWorkflowSequenceOnlineNode.cxx
  vtkMRMLWorkflowSequenceOnlineNode.h
  
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

# --------------------------------------------------------------------------
# Build the library

set(${KIT}_TARGET_LIBRARIES
  vtkSlicerSequencesModuleMRML
  vtkSlicerSequenceBrowserModuleMRML
  )

SlicerMacroBuildModuleMRML(
  NAME ${KIT}
  EXPORT_DIRECTIVE ${${KIT}_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${${KIT}_INCLUDE_DIRECTORIES}
  SRCS ${${KIT}_SRCS}
  TARGET_LIBRARIES ${${KIT}_TARGET_LIBRARIES}
  )