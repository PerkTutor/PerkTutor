cmake_minimum_required(VERSION 2.8.9)

#-----------------------------------------------------------------------------
set(MODULE_NAME PythonMetricsCalculator)

#Change this once it is packaged as an extension
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

#-----------------------------------------------------------------------------
# Update the Python Metrics git submodule
execute_process(
  COMMAND ${GIT_EXECUTABLE} submodule update
)

#-----------------------------------------------------------------------------
set(PERKTUTOR_CORE_METRICS
  PythonMetrics/ElapsedTime.py
  PythonMetrics/PathLength.py
  PythonMetrics/TissueDamage.py
  PythonMetrics/TissuePath.py
  PythonMetrics/TissueTime.py
  )


set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  ${PERKTUTOR_CORE_METRICS}
  )

#-----------------------------------------------------------------------------
set(SCENE_TEST_RESOURCES
  Data/Lumbar/BodyModel.stl
  Data/Lumbar/StylusTipRotation.tfm
  Data/Lumbar/ProbeToRAS.tfm
  Data/Lumbar/StylusTipToRAS.tfm
  Data/Lumbar/TransformBuffer_Lumbar_Anonymous.xml
  Data/Lumbar/TrueMetricsTable.tsv
  Data/Lumbar/Scene_Lumbar.mrml
  )


#-----------------------------------------------------------------------------
set(MODULE_PYTHON_RESOURCES
  Resources/${MODULE_NAME}.qrc
  )

if (BUILD_TESTING)
  set(MODULE_PYTHON_RESOURCES
    ${MODULE_PYTHON_RESOURCES}
    ${SCENE_TEST_RESOURCES}
    )
endif()

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  WITH_GENERIC_TESTS # Note: The generic test always fails because the scripted module cannot find the loadable modules it depends on. This is expected.
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)

  # Register the unittest subclass in the main script as a ctest.
  # Note that the test will also be available at runtime.
  slicer_add_python_unittest(
    SCRIPT ${MODULE_NAME}.py
    SLICER_ARGS --additional-module-paths
      ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}
      ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/Debug
      ${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/Release
      ${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
	  ${Sequences_LIB_DIR}
    )
  # We need to add these additional module paths, because they are not otherwise found correctly

  # Additional build-time testing
  add_subdirectory(Testing)
endif()

