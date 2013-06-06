
#ifndef __vtkWorkflowProcedure_h
#define __vtkWorkflowProcedure_h

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
#include "vtkWorkflowTask.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkWorkflowProcedure : public vtkObject
{
public:
  vtkTypeMacro( vtkWorkflowProcedure, vtkObject );

  // Standard MRML methods
  static vtkWorkflowProcedure* New();

  vtkWorkflowProcedure* DeepCopy();

protected:

  // Constructo/destructor
  vtkWorkflowProcedure();
  virtual ~vtkWorkflowProcedure();

public:

  int GetNumTasks();
  vtkWorkflowTask* GetTaskAt( int index );
  vtkWorkflowTask* GetTaskByName( std::string name );
  std::vector<std::string> GetTaskNames();

  void AddTask( vtkWorkflowTask* newTask );

  bool IsTask( std::string name );
  int IndexByName( std::string name );

  std::string ToXMLString();
  void FromXMLElement( vtkXMLDataElement* element );

private:

  std::vector<vtkWorkflowTask*> Tasks;

public:

  std::string Name;

};

#endif