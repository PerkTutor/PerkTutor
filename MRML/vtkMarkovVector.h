
#ifndef __vtkMarkovVector_h
#define __vtkMarkovVector_h

// Standard Includes
#include <string>
#include <sstream>
#include <vector>
#include <cmath>

// VTK includes
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"
#include "vtkXMLDataElement.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkMarkovVector : public vtkObject
{
public:
  vtkTypeMacro( vtkMarkovVector, vtkObject );

  // Standard MRML methods
  static vtkMarkovVector* New();

protected:

  // Constructo/destructor
  vtkMarkovVector();
  virtual ~vtkMarkovVector();

public:

  //
  void Copy( vtkMarkovVector* otherVector );

  std::string GetState();
  void SetState( std::string newState );
  void SetState( int newState );

  std::string GetSymbol();
  void SetSymbol( std::string newSymbol );
  void SetSymbol( int newSymbol );

  std::string ToXMLString( vtkIndent indent );
  void FromXMLElement( vtkXMLDataElement* element );
  
protected:
  std::string State;
  std::string Symbol;


};

#endif