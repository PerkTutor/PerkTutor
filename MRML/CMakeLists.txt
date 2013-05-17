project(vtkSlicer${MODULE_NAME}ModuleMRML)

set(KIT ${PROJECT_NAME})

set(${KIT}_EXPORT_DIRECTIVE "VTK_SLICER_${MODULE_NAME_UPPER}_MODULE_MRML_EXPORT")

set(${KIT}_INCLUDE_DIRECTORIES
  ${Slicer_Base_INCLUDE_DIRS}
  )

# --------------------------------------------------------------------------
# Sources

set(${KIT}_SRCS
  vtkMRMLWorkflowSegmentationNode.cxx
  vtkMRMLWorkflowSegmentationNode.h
  vtkLabelRecord.cxx
  vtkLabelRecord.h
  vtkLabelVector.cxx
  vtkLabelVector.h
  vtkMarkovModel.cxx
  vtkMarkovModel.h
  vtkMarkovModelRT.cxx
  vtkMarkovModelRT.h
  vtkMarkovRecord.cxx
  vtkMarkovRecord.h
  vtkRecordBuffer.cxx
  vtkRecordBuffer.h
  vtkRecordBufferRT.cxx
  vtkRecordBufferRT.h
  vtkTrackingRecord.cxx
  vtkTrackingRecord.h
  vtkWorkflowInput.cxx
  vtkWorkflowInput.h
  vtkWorkflowProcedure.cxx
  vtkWorkflowProcedure.h
  vtkWorkflowTask.cxx
  vtkWorkflowTask.h
  vtkWorkflowTool.cxx
  vtkWorkflowTool.h
  vtkWorkflowToolCollection.cxx
  vtkWorkflowToolCollection.h
  vtkWorkflowTraining.cxx
  vtkWorkflowTraining.h
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