
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
#include "vtkRecordBuffer.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkWorkflowTool : public vtkObject
{
public:
  vtkTypeMacro( vtkWorkflowTool, vtkObject );

  // Standard MRML methods
  static vtkWorkflowTool* New();

  vtkWorkflowTool* DeepCopy();

protected:

  // Constructo/destructor
  vtkWorkflowTool();
  virtual ~vtkWorkflowTool();

public:

  std::string Name;
  bool Defined, Inputted, Trained;

  vtkWorkflowProcedure* Procedure;
  vtkWorkflowInput* Input;
  vtkWorkflowTraining* Training;
  vtkRecordBuffer* Buffer;

  std::string ProcedureToXMLString();
  void ProcedureFromXMLElement( vtkXMLDataElement* element );

  std::string InputToXMLString();
  void InputFromXMLElement( vtkXMLDataElement* element );

  std::string TrainingToXMLString();
  void TrainingFromXMLElement( vtkXMLDataElement* element );

};

#endif