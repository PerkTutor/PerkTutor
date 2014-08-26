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

#-----------------------------------------------------------------------------
set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  ${PERKTUTOR_CORE_METRICS}
  )

set(MODULE_PYTHON_RESOURCES
  Resources/${MODULE_NAME}.qrc
  )

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
  slicer_add_python_unittest(SCRIPT ${MODULE_NAME}.py)

  # Additional build-time testing
  add_subdirectory(Testing)
endif()

