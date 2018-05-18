
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
#include "vtkCommand.h"
#include "vtkNew.h"
#include "vtkCollection.h"
#include "vtkCollectionIterator.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkMRMLWorkflowProcedureNode.h"
#include "vtkMRMLWorkflowInputNode.h"
#include "vtkMRMLWorkflowTrainingNode.h"
#include "vtkMRMLWorkflowSequenceNode.h"
#include "vtkMRMLWorkflowSequenceOnlineNode.h"

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

  // Getters/setters
  vtkGetMacro( ToolName, std::string );
  vtkSetMacro( ToolName, std::string );
  
  bool IsWorkflowProcedureSet();
  bool IsWorkflowInputSet();
  bool IsWorkflowTrainingSet();
  
  std::string GetNodeReferenceIDString( std::string referenceRole );

  vtkMRMLLinearTransformNode* GetToolTransformNode();
  std::string GetToolTransformID();
  void SetToolTransformID( std::string newToolTransformID );

  vtkMRMLWorkflowProcedureNode* GetWorkflowProcedureNode();
  std::string GetWorkflowProcedureID();
  void SetWorkflowProcedureID( std::string newWorkflowProcedureID );
  
  vtkMRMLWorkflowInputNode* GetWorkflowInputNode();
  std::string GetWorkflowInputID();
  void SetWorkflowInputID( std::string newWorkflowInputID );
  
  vtkMRMLWorkflowTrainingNode* GetWorkflowTrainingNode();
  std::string GetWorkflowTrainingID();
  void SetWorkflowTrainingID( std::string newWorkflowTrainingID );

  vtkWorkflowTask* GetCurrentTask();
  void SetCurrentTask( vtkWorkflowTask* newCurrentTask );
  
  
  // Computation
  void ResetWorkflowSequences();
  
  bool Train( vtkCollection* trainingWorkflowSequences );
  
  void AddAndSegmentTransform( vtkMRMLLinearTransformNode* newTransform, std::string newTimeString );
  

  // Propagate the modified event from any of the tools
  virtual void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );


  // Events
  enum
  {
    CurrentTaskChangedEvent = vtkCommand::UserEvent + 1,
  };

protected:

  std::string ToolName;
  
  // This will also hold the real-time workflow sequences
  vtkSmartPointer< vtkMRMLWorkflowSequenceOnlineNode > RawWorkflowSequence, FilterWorkflowSequence, DerivativeWorkflowSequence, OrthogonalWorkflowSequence, PcaWorkflowSequence, CentroidWorkflowSequence;
  vtkSmartPointer< vtkWorkflowTask > CurrentTask;

  bool CurrentTaskNew;
  
  // Internal helpers for computation
  std::map< std::string, double > CalculateTaskProportions( vtkCollection* trainingWorkflowSequences );
  std::map< std::string, double > EqualizeTaskProportions( vtkCollection* trainingWorkflowSequences );
  std::map< std::string, int > CalculateTaskNumCentroids( vtkCollection* trainingWorkflowSequences );
};

#endif