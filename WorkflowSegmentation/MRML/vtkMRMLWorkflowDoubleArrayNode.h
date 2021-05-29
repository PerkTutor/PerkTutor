
#ifndef __vtkMRMLWorkflowDoubleArrayNode_h
#define __vtkMRMLWorkflowDoubleArrayNode_h

// Standard Includes
#include <string>
#include <sstream>
#include <vector>
#include <cmath>

// VTK includes
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"
#include "vtkDoubleArray.h"

#include "vtkMRMLNode.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkMRMLWorkflowDoubleArrayNode : public vtkMRMLNode
{
public:
  vtkTypeMacro( vtkMRMLWorkflowDoubleArrayNode, vtkMRMLNode );

  // Standard MRML node methods  
  static vtkMRMLWorkflowDoubleArrayNode* New();
  vtkMRMLNode* CreateNodeInstance() override;
  virtual const char* GetNodeTagName() override { return "WorkflowDoubleArray"; };

  vtkMRMLCopyContentMacro(vtkMRMLWorkflowDoubleArrayNode);
  
protected:

  // Constructor/desctructor
  vtkMRMLWorkflowDoubleArrayNode();
  virtual ~vtkMRMLWorkflowDoubleArrayNode();
  vtkMRMLWorkflowDoubleArrayNode ( const vtkMRMLWorkflowDoubleArrayNode& ); // Required to prevent linking error
  void operator=( const vtkMRMLWorkflowDoubleArrayNode& ); // Required to prevent linking error

public:
  // Getters/setters for properties
  virtual void SetArray(vtkDoubleArray*);
  vtkGetObjectMacro(Array, vtkDoubleArray);
  
protected:
  vtkDoubleArray* Array;	// The array
};

#endif