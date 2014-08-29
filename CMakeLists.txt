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
  PythonMetrics/__init__.py
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
  Data/Scenes/Lumbar/BodyModel.stl
  Data/Scenes/Lumbar/NeedleRotation.tfm
  Data/Scenes/Lumbar/ProbeToRAS.tfm
  Data/Scenes/Lumbar/StylusTipToRAS.tfm
  Data/Scenes/Lumbar/TransformBuffer_Subject13.xml
  Data/Scenes/Lumbar/TransformBuffer_Lumbar_Scene.mrml  
  )
  
set(RESULTS_TEST_RESOURCES
  Data/Results/Lumbar.xml
  )

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_RESOURCES
  Resources/${MODULE_NAME}.qrc
  )

if (BUILD_TESTING)
  set(MODULE_PYTHON_RESOURCES
    ${MODULE_PYTHON_RESOURCES}
    ${SCENE_TEST_RESOURCES}
    ${RESULTS_TEST_RESOURCES}
    )
endif()

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  WITH_GENERIC_TESTS
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
    )

  # Additional build-time testing
  add_subdirectory(Testing)
endif()

