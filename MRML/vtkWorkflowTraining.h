
#ifndef __vtkWorkflowTraining_h
#define __vtkWorkflowTraining_h

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
#include "vtkTrackingRecord.h"
#include "vtkMarkovModelRT.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkWorkflowTraining : public vtkObject
{
public:
  vtkTypeMacro( vtkWorkflowTraining, vtkObject );

  // Standard MRML methods
  static vtkWorkflowTraining* New();

  vtkWorkflowTraining* DeepCopy();

protected:

  // Constructo/destructor
  vtkWorkflowTraining();
  virtual ~vtkWorkflowTraining();

public:

  std::vector<vtkLabelVector*> PrinComps;
  vtkLabelVector* Mean;
  std::vector<vtkLabelVector*> Centroids;
  vtkMarkovModelRT* Markov;

  std::string ToXMLString();
  void FromXMLElement( vtkXMLDataElement* element );

};

#endif