
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

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkWorkflowTask.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkMRMLWorkflowProcedureNode : public vtkMRMLStorableNode
{
public:
  vtkTypeMacro( vtkMRMLWorkflowInputNode, vtkMRMLStorableNode );

  // Standard MRML node methods  
  static vtkMRMLWorkflowProcedureNode* New();  
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "WorkflowProcedure"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );

  // To use the storage node
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode() { return vtkMRMLWorkflowProcedureStorageNode::New(); };
  bool GetModifiedSinceRead() { return ( this->GetMTime() > this->GetStoredTime() ); };
  virtual void UpdateScene( vtkMRMLScene *scene ) { Superclass::UpdateScene(scene); };
  
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
  vtkGetMacro( Name, std::string );
  vtkSetMacro( Name, std::string );

  std::string ToXMLString( vtkIndent indent );
  void FromXMLElement( vtkXMLDataElement* element );

protected:

  std::map< std::string, vtkSmartPointer< vtkWorkflowTask > > Tasks;

};

#endif