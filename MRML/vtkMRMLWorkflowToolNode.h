
#ifndef __vtkMRMLWorkflowToolNode_h
#define __vtkMRMLWorkflowToolNode_h

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
#include "vtkMRMLNode.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkMRMLWorkflowToolNode.h"
#include "vtkWorkflowInput.h"
#include "vtkWorkflowTraining.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkMRMLWorkflowToolNode : public vtkMRMLNode
{
public:
  vtkTypeMacro( vtkMRMLWorkflowToolNode, vtkMRMLNode );

  // Standard MRML node methods  
  static vtkMRMLWorkflowToolNode* New();  
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "WorkflowTool"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );
  
protected:

  // Constructor/desctructor
  vtkMRMLWorkflowToolNode();
  virtual ~vtkMRMLWorkflowToolNode();
  vtkMRMLWorkflowToolNode ( const vtkMRMLWorkflowToolNode& ); // Required to prevent linking error
  void operator=( const vtkMRMLWorkflowToolNode& ); // Required to prevent linking error

  
public:

  vtkGetMacro( Name, std::string );
  vtkSetMacro( Name, std::string );
  
  bool GetDefined();
  bool GetInputted();
  bool GetTrained();
  
  std::string GetNodeReferenceIDString( std::string referenceRole );

  vtkMRMLWorkflowProcedureNode* GetWorkflowProcedureNode();
  std::string GetWorkflowProcedureID();
  void SetWorkflowProcrdureID( std::string newWorkflowProcedureID );
  
  vtkMRMLWorkflowInputNode* GetWorkflowInputNode();
  std::string GetWorkflowInputID();
  void SetWorkflowInputID( std::string newWorkflowInputID );
  
  vtkMRMLWorkflowTrainingNode* GetWorkflowTrainingNode();
  std::string GetWorkflowTrainingID();
  void SetWorkflowTrainingID( std::string newWorkflowTrainingID );

protected:

  std::string Name;
  bool Defined, Inputted, Trained;

};

#endif