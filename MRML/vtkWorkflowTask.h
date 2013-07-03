
#ifndef __vtkWorkflowTask_h
#define __vtkWorkflowTask_h

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
#include "vtkLabelVector.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkWorkflowTask : public vtkObject
{
public:
  vtkTypeMacro( vtkWorkflowTask, vtkObject );

  // Standard MRML methods
  static vtkWorkflowTask* New();

  vtkWorkflowTask* DeepCopy();

protected:

  // Constructo/destructor
  vtkWorkflowTask();
  virtual ~vtkWorkflowTask();

public:

  std::string Name;
  std::string Instruction;
  std::string Next;
  std::string Prerequisite;
  std::string Recovery;


public:

  std::string ToXMLString();
  void FromXMLElement( vtkXMLDataElement* element );

};

#endif