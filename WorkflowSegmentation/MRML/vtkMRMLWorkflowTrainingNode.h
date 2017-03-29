
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

#include "vtkMRMLWorkflowTrainingStorageNode.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkMarkovModelOnline.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkMRMLWorkflowTrainingNode : public vtkMRMLStorableNode
{
public:
  vtkTypeMacro( vtkMRMLWorkflowTrainingNode, vtkMRMLStorableNode );

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
  vtkDoubleArray* GetMean();
  void SetMean( vtkDoubleArray* newMean );

  vtkDoubleArray* GetPrinComps();
  void SetPrinComps( vtkDoubleArray* newPrinComps );

  vtkDoubleArray* GetCentroids();
  void SetCentroids( vtkDoubleArray* newCentroids );

  vtkMarkovModelOnline* GetMarkov();
  void SetMarkov( vtkMarkovModelOnline* newMarkov );
  
  // Read/Write from/to file
  std::string ToXMLString( vtkIndent indent );
  void FromXMLElement( vtkXMLDataElement* element );

  // Static methods to read/write double arrays
  static std::string DoubleArrayToXMLString( vtkDoubleArray* matrix, std::string name, vtkIndent indent );
  static void DoubleArrayFromXMLElement( vtkXMLDataElement* element, std::string name, vtkDoubleArray* matrix );  

protected:

  vtkSmartPointer< vtkDoubleArray > Mean;

  vtkSmartPointer< vtkDoubleArray > PrinComps;
  vtkSmartPointer< vtkDoubleArray > Centroids;  
  
  vtkSmartPointer< vtkMarkovModelOnline > Markov;
};

#endif