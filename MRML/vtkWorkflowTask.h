
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

protected:

  // Constructo/destructor
  vtkWorkflowTask();
  virtual ~vtkWorkflowTask();


public:

  // 
  void Copy( vtkWorkflowTask* otherTask );

  // Getters/setters for properties
  vtkGetMacro( Name, std::string );
  vtkSetMacro( Name, std::string );

  vtkGetMacro( Instruction, std::string );
  vtkSetMacro( Instruction, std::string );

  vtkGetMacro( Next, std::string );
  vtkSetMacro( Next, std::string );

  vtkGetMacro( Prerequisite, std::string );
  vtkSetMacro( Prerequisite, std::string );

  vtkGetMacro( Recovery, std::string );
  vtkSetMacro( Recovery, std::string );


  // Read/write task to file
  std::string ToXMLString( vtkIndent indent );
  void FromXMLElement( vtkXMLDataElement* element );


protected:

  std::string Name;
  std::string Instruction;
  std::string Next;
  std::string Prerequisite;
  std::string Recovery;

};

#endif