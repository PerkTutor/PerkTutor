set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkSlicerPythonMetricsTest.cxx
  )

#-----------------------------------------------------------------------------
QT4_GENERATE_MOCS(
  )
  
#-----------------------------------------------------------------------------
set(TEST_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleLogic
  vtkSlicerTransformRecorderModuleMRML
  )

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  )


#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES ${TEST_TARGET_LIBRARIES}
  WITH_VTK_DEBUG_LEAKS_CHECK
  )

#-----------------------------------------------------------------------------
# The "core" metrics are time, path length, tissue time, tissue path, tissue damage
# We only attempt to test the "core" metrics
macro(TEST_WITH_DATA 
      TestName 
      TestExecutableName
      SceneFile      
      TransformBufferID
      TissueModelID
      NeedleTransformID
      ScriptedMetricsDirectory
      ExpectedResultsFile
      PythonBinaryDirectory      )
  message( ${Slicer_LAUNCH_COMMAND} )
  add_test(
    NAME ${TestName}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TestExecutableName} ${ARGN}
    -SceneFile ${SceneFile}    
    -TransformBufferID ${TransformBufferID}
    -TissueModelID ${TissueModelID}
    -NeedleTransformID ${NeedleTransformID}
    -ScriptedMetricsDirectory ${ScriptedMetricsDirectory}
    -ExpectedResultsFile ${ExpectedResultsFile}
    -PythonBinaryDirectory ${PythonBinaryDirectory}
  )
endmacro()

#-----------------------------------------------------------------------------
TEST_WITH_DATA(
  vtkSlicerPythonMetricsTest_Lumbar
  vtkSlicerPythonMetricsTest
  ${CMAKE_CURRENT_SOURCE_DIR}/../Scenes/Lumbar/TransformBuffer_Lumbar_Scene.mrml
  vtkMRMLTransformBufferNode1
  vtkMRMLModelNode1
  vtkMRMLLinearTransformNode1
  ${CMAKE_CURRENT_SOURCE_DIR}/../Metrics
  ${CMAKE_CURRENT_SOURCE_DIR}/../Results/Lumbar.xml
  ${${EXTENSION_NAME}_BINARY_DIR}/lib/Slicer-${Slicer_VERSION}/qt-loadable-modules/Python
)
set_tests_properties(vtkSlicerPythonMetricsTest_Lumbar PROPERTIES FAIL_REGULAR_EXPRESSION "Error;ERROR;Warning;WARNING" ENVIRONMENT PythonMetricsTestDataPath=${CMAKE_CURRENT_SOURCE_DIR})