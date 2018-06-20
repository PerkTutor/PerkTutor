# Remove once packaged in an extension
cmake_minimum_required(VERSION 2.8.9)


#-----------------------------------------------------------------------------
set(MODULE_NAME SkillAssessment)
string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

# Remove once packaged in an extension
find_package(Slicer REQUIRED)
include(${Slicer_USE_FILE})

include(ExternalProject)

#-----------------------------------------------------------------------------
set(ASSESSMENT_METHODS_SCRIPTS
  AssessmentMethods/__init__.py
  AssessmentMethods/Benchmark.py
  AssessmentMethods/DecisionTree.py
  AssessmentMethods/Fuzzy.py
  AssessmentMethods/LinearCombination.py
  AssessmentMethods/NearestNeighbor.py
  AssessmentMethods/Regression.py
  AssessmentMethods/FuzzyLogic/__init__.py
  AssessmentMethods/FuzzyLogic/BinaryFunction.py
  AssessmentMethods/FuzzyLogic/Defuzzifier.py
  AssessmentMethods/FuzzyLogic/FuzzyRule.py
  AssessmentMethods/FuzzyLogic/MembershipFunction.py
)

set(MODULE_PYTHON_SCRIPTS
  ${MODULE_NAME}.py
  ${ASSESSMENT_METHODS_SCRIPTS}
  )

set(MODULE_PYTHON_RESOURCES
  Resources/Icons/${MODULE_NAME}.png
  )

#-----------------------------------------------------------------------------
slicerMacroBuildScriptedModule(
  NAME ${MODULE_NAME}
  SCRIPTS ${MODULE_PYTHON_SCRIPTS}
  RESOURCES ${MODULE_PYTHON_RESOURCES}
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)

  # Register the unittest subclass in the main script as a ctest.
  # Note that the test will also be available at runtime.
  slicer_add_python_unittest(SCRIPT ${MODULE_NAME}.py)

  # Additional build-time testing
  add_subdirectory(Testing)
endif()
