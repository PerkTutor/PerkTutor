# TODO: Is there a more elegant way to set these?


# Set the include directories
set( Sequences_INCLUDE_DIRS 
  "${Sequences_SOURCE_DIR}/Sequences/MRML"
  "${Sequences_BINARY_DIR}/Sequences/MRML"
  "${Sequences_SOURCE_DIR}/Sequences/Logic"
  "${Sequences_BINARY_DIR}/Sequences/Logic"
  "${Sequences_SOURCE_DIR}/Sequences/Widgets"
  "${Sequences_BINARY_DIR}/Sequences/Widgets"
  "${Sequences_SOURCE_DIR}/SequenceBrowser/MRML"
  "${Sequences_BINARY_DIR}/SequenceBrowser/MRML"
  "${Sequences_SOURCE_DIR}/SequenceBrowser/Logic"
  "${Sequences_BINARY_DIR}/SequenceBrowser/Logic"
  "${Sequences_SOURCE_DIR}/SequenceBrowser/Widgets"
  "${Sequences_BINARY_DIR}/SequenceBrowser/Widgets"
  )
  
# Grab the library files
file( GLOB Sequences_LIB_DIR
  "${Sequences_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/*"
  )
set( Sequences_LIB_NAMES
  qSlicerSequencesModuleWidgets
  qSlicerSequenceBrowserModuleWidgets
  vtkSlicerSequenceBrowserModuleMRML
  vtkSlicerSequencesModuleMRML
  vtkSlicerSequencesModuleMRMLPython # Needed for subclassing
  vtkSlicerSequencesModuleMRMLPythonD # Needed for subclassing
  vtkSlicerSequenceBrowserModuleLogic
  vtkSlicerSequencesModuleLogic
  )

# Find and add the libraries
foreach( lib_name ${Sequences_LIB_NAMES} )

  unset( CURR_LIB CACHE )
  find_library( CURR_LIB
    NAME ${lib_name}
    PATHS "${Sequences_LIB_DIR}"
    )
    
  set( Sequences_LIBS
    ${Sequences_LIBS}
    ${CURR_LIB}
    )
endforeach( lib_name )


# To use the sequences extension in a module, add:
# Sequences_INCLUDE_DIRS to the INCLUDE_DIRECTORIES
# Sequences_LIBS to the TARGET_LIBRARIES