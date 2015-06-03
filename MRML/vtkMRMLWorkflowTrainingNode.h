
#ifndef __vtkMRMLWorkflowTrainingNode_h
#define __vtkMRMLWorkflowTrainingNode_h

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
#include "vtkMRMLStorageNode.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkTrackingRecord.h"
#include "vtkMarkovModelRT.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkMRMLWorkflowTrainingNode : public vtkMRMLStorableNode
{
public:
  vtkTypeMacro( vtkMRMLWorkflowInputNode, vtkMRMLStorableNode );

  // Standard MRML node methods  
  static vtkMRMLWorkflowTrainingNode* New();  
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "WorkflowTraining"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );

  // To use the storage node
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode() { return vtkMRMLWorkflowTrainingStorageNode::New(); };
  bool GetModifiedSinceRead() { return ( this->GetMTime() > this->GetStoredTime() ); };
  virtual void UpdateScene( vtkMRMLScene *scene ) { Superclass::UpdateScene(scene); };
  
protected:

  // Constructor/desctructor
  vtkMRMLWorkflowTrainingNode();
  virtual ~vtkMRMLWorkflowTrainingNode();
  vtkMRMLWorkflowTrainingNode ( const vtkMRMLWorkflowTrainingNode& ); // Required to prevent linking error
  void operator=( const vtkMRMLWorkflowTrainingNode& ); // Required to prevent linking error
  
public:

  // Getters/setters for properties
  vtkGetMacro( PrinComps, std::vector< vtkLabelVector* > );
  vtkSetMacro( PrinComps, std::vector< vtkLabelVector* > );
  
  vtkGetMacro( Centroids, std::vector< vtkLabelVector* > );
  vtkSetMacro( Centroids, std::vector< vtkLabelVector* > );
  
  vtkGetMacro( Mean, vtkLabelVector* );
  vtkSetMacro( Mean, vtkLabelVector* );
  
  vtkGetMacro( Markov, vtkMarkovModelRT* );
  vtkSetMacro( Markov, vtkMarkovModelRT* );
  
  // Read/Write from/to file
  std::string ToXMLString( vtkIndent indent );
  void FromXMLElement( vtkXMLDataElement* element );

protected:

  std::vector< vtkSmartPointer< vtkLabelVector > > PrinComps;
  std::vector< vtkSmartPointer< vtkLabelVector > > Centroids;
  
  vtkSmartPointer< vtkLabelVector > Mean;
  vtkSmartPointer< vtkMarkovModelRT > Markov;

};

#endif