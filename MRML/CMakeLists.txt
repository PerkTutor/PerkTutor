
project(vtkSlicer${MODULE_NAME}ModuleMRML)

#
# See CMake/SlicerMacroBuildModuleMRML.cmake for details
#

set(module_mrml_name "${PROJECT_NAME}")

# The header '${module_mrml_name}Export.h' will be automatically configured.
set(module_mrml_export_directive "VTK_SLICER_TRANSFORMRECORDER_MODULE_MRML_EXPORT")

# Debug
#MESSAGE( STATUS "TransformRecorder ====================================" )
#MESSAGE( STATUS "Included: ${Slicer_BINARY_DIR}/Modules/Loadable/OpenIGTLinkIF" )



# Additional directories to include
set(module_mrml_include_directories
  ${Slicer_Base_INCLUDE_DIRS}
  #${Slicer_BINARY_DIR}/Modules/Loadable/OpenIGTLinkIF/MRML
  )

# Source files
set(module_mrml_SRCS
  vtkMRMLTransformBufferNode.cxx
  vtkMRMLTransformBufferNode.h
  vtkMRMLTransformBufferStorageNode.cxx
  vtkMRMLTransformBufferStorageNode.h
  vtkLogRecordBuffer.cxx
  vtkLogRecordBuffer.h
  vtkLogRecord.h
  vtkLogRecord.cxx
  vtkTransformRecord.cxx
  vtkTransformRecord.h
  vtkMessageRecord.cxx
  vtkMessageRecord.h
  )

# Additional Target libraries
set(module_mrml_target_libraries
  ${ITK_LIBRARIES}
  ${MRML_LIBRARIES}
  SlicerBaseLogic
  #vtkSlicerOpenIGTLinkIFModuleMRML
  vtkSlicerAnnotationsModuleMRML 
  )

SlicerMacroBuildModuleMRML(
  NAME ${module_mrml_name}
  EXPORT_DIRECTIVE ${module_mrml_export_directive}
  INCLUDE_DIRECTORIES ${module_mrml_include_directories}
  SRCS ${module_mrml_SRCS}
  TARGET_LIBRARIES ${module_mrml_target_libraries}
  )