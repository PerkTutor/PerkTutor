
#ifndef __vtkWorkflowInput_h
#define __vtkWorkflowInput_h

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
vtkWorkflowInput : public vtkObject
{
public:
  vtkTypeMacro( vtkWorkflowInput, vtkObject );

  // Standard MRML methods
  static vtkWorkflowInput* New();

  vtkWorkflowInput* DeepCopy();

protected:

  // Constructo/destructor
  vtkWorkflowInput();
  virtual ~vtkWorkflowInput();

public:

  double FilterWidth;
  int OrthogonalOrder;
  int OrthogonalWindow;
  int Derivative;
  int NumCentroids;
  int NumPrinComps;
  double MarkovPseudoScalePi;
  double MarkovPseudoScaleA;
  double MarkovPseudoScaleB;
  double CompletionTime;
  double Equalization;

  std::string ToXMLString();
  void FromXMLElement( vtkXMLDataElement* element );

};

#endif