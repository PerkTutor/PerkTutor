
#ifndef __vtkMarkovRecord_h
#define __vtkMarkovRecord_h

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
vtkMarkovRecord : public vtkObject
{
public:
  vtkTypeMacro( vtkMarkovRecord, vtkObject );

  // Standard MRML methods
  static vtkMarkovRecord* New();

  vtkMarkovRecord* DeepCopy();

protected:

  // Constructo/destructor
  vtkMarkovRecord();
  virtual ~vtkMarkovRecord();

private:
  std::string State;
  std::string Symbol;

public:

  std::string GetState();
  void SetState( std::string newState );
  void SetState( int newState );

  std::string GetSymbol();
  void SetSymbol( std::string newSymbol );
  void SetSymbol( int newSymbol );

  std::string ToXMLString();
  void FromXMLElement( vtkXMLDataElement* element );


};

#endif