
#ifndef __vtkWorkflowTool_h
#define __vtkWorkflowTool_h

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
#include "vtkWorkflowProcedure.h"
#include "vtkWorkflowInput.h"
#include "vtkWorkflowTraining.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkWorkflowTool : public vtkObject
{
public:
  vtkTypMacro( vtkWorkflowTool, vtkObject );

  // Standard MRML methods
  static vtkWorkflowTool* New();

protected:

  // Constructo/destructor
  vtkWorkflowTool();
  virtual ~vtkWorkflowTool();

private:

  std::string Name;
  bool Defined, Inputted, Trained;

public:

  vtkWorkflowProcedure* Procedure;
  vtkWorkflowInput* Input;
  vtkWorkflowTraining* Training;
  vtkRecordBuffer* Buffer;

  std::string WorkflowProcedureToXMLString();
  void WorkflowProcedureFromXMLElement( vtkXMLDataElement* element );

  std::string InputParameterToXMLString();
  void InputParameterFromXMLElement( vtkXMLDataElement* element );

  std::string TrainingParameterToXMLString();
  void TrainingParameterFromXMLElement( vtkXMLDataElement* element );

};

#endif