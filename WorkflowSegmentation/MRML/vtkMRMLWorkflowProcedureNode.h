
#ifndef __vtkMRMLWorkflowProcedureNode_h
#define __vtkMRMLWorkflowProcedureNode_h

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

#include "vtkMRMLStorableNode.h"
#include "vtkMRMLWorkflowProcedureStorageNode.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkWorkflowTask.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkMRMLWorkflowProcedureNode : public vtkMRMLStorableNode
{
public:
  vtkTypeMacro( vtkMRMLWorkflowProcedureNode, vtkMRMLStorableNode );

  // Standard MRML node methods  
  static vtkMRMLWorkflowProcedureNode* New();  
  vtkMRMLNode* CreateNodeInstance() override;
  const char* GetNodeTagName() override { return "WorkflowProcedure"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  void ReadXMLAttributes( const char** atts ) override;
  void WriteXML( ostream& of, int indent ) override;
  void Copy( vtkMRMLNode *node ) override;

  // To use the storage node
  vtkMRMLStorageNode* CreateDefaultStorageNode() override { return vtkMRMLWorkflowProcedureStorageNode::New(); };
  bool GetModifiedSinceRead() { return ( this->GetMTime() > this->GetStoredTime() ); };
  void UpdateScene( vtkMRMLScene *scene ) override { Superclass::UpdateScene(scene); };
  
protected:

  // Constructor/desctructor
  vtkMRMLWorkflowProcedureNode();
  virtual ~vtkMRMLWorkflowProcedureNode();
  vtkMRMLWorkflowProcedureNode ( const vtkMRMLWorkflowProcedureNode& ); // Required to prevent linking error
  void operator=( const vtkMRMLWorkflowProcedureNode& ); // Required to prevent linking error

public:

  int GetNumTasks();
  vtkWorkflowTask* GetTask( std::string name );
  std::vector< std::string > GetAllTaskNames();

  void AddTask( vtkWorkflowTask* newTask );

  bool IsTask( std::string name );
  
  // Getters/setters for properties
  vtkGetMacro( ProcedureName, std::string );
  vtkSetMacro( ProcedureName, std::string );

  std::string ToXMLString( vtkIndent indent );
  void FromXMLElement( vtkXMLDataElement* element );

protected:

  std::map< std::string, vtkSmartPointer< vtkWorkflowTask > > Tasks;

  std::string ProcedureName;

};

#endif