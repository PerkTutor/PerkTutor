#### Temporary to get build working ####
cmake_minimum_required( VERSION 2.8.7 )

project( FuzzySkillEvaluator )
find_package( Slicer REQUIRED )
include( ${Slicer_USE_FILE} )

#### Temporary to get build working ####


#-----------------------------------------------------------------------------
set(MODULE_NAME FuzzySkillEvaluator)

#-----------------------------------------------------------------------------
set(FUZZY_LOGIC_PYTHON_SCRIPTS
  FuzzyLogic/__init__.py
  FuzzyLogic/MembershipFunction.py
  FuzzyLogic/BinaryFunction.py
  FuzzyLogic/FuzzyRule.py
  FuzzyLogic/Defuzzifier.py
  )

set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  ${FUZZY_LOGIC_PYTHON_SCRIPTS}
  )

set(MODULE_PYTHON_RESOURCES
  Resources/Icons/${MODULE_NAME}.png
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

