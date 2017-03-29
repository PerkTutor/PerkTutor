
#ifndef __vtkMRMLWorkflowInputNode_h
#define __vtkMRMLWorkflowInputNode_h

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

#include "vtkMRMLWorkflowInputStorageNode.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"

// This class stores a vector of values and a string label
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkMRMLWorkflowInputNode : public vtkMRMLStorableNode
{
public:
  vtkTypeMacro( vtkMRMLWorkflowInputNode, vtkMRMLStorableNode );

  // Standard MRML node methods  
  static vtkMRMLWorkflowInputNode* New();  
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "WorkflowInput"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );

  // To use the storage node
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode() { return vtkMRMLWorkflowInputStorageNode::New(); };
  bool GetModifiedSinceRead() { return ( this->GetMTime() > this->GetStoredTime() ); };
  virtual void UpdateScene( vtkMRMLScene *scene ) { Superclass::UpdateScene(scene); };
  
protected:

  // Constructor/desctructor
  vtkMRMLWorkflowInputNode();
  virtual ~vtkMRMLWorkflowInputNode();
  vtkMRMLWorkflowInputNode ( const vtkMRMLWorkflowInputNode& ); // Required to prevent linking error
  void operator=( const vtkMRMLWorkflowInputNode& ); // Required to prevent linking error

public:

  // Getters/setters for properties
  vtkGetMacro( FilterWidth, double );
  vtkSetMacro( FilterWidth, double );

  vtkGetMacro( OrthogonalOrder, int );
  vtkSetMacro( OrthogonalOrder, int );

  vtkGetMacro( OrthogonalWindow, int );
  vtkSetMacro( OrthogonalWindow, int );

  vtkGetMacro( Derivative, int );
  vtkSetMacro( Derivative, int );

  vtkGetMacro( NumCentroids, int );
  vtkSetMacro( NumCentroids, int );

  vtkGetMacro( NumPrinComps, int );
  vtkSetMacro( NumPrinComps, int );

  vtkGetMacro( MarkovPseudoScalePi, double );
  vtkSetMacro( MarkovPseudoScalePi, double );

  vtkGetMacro( MarkovPseudoScaleA, double );
  vtkSetMacro( MarkovPseudoScaleA, double );

  vtkGetMacro( MarkovPseudoScaleB, double );
  vtkSetMacro( MarkovPseudoScaleB, double );

  vtkGetMacro( CompletionTime, double );
  vtkSetMacro( CompletionTime, double );

  vtkGetMacro( Equalization, double );
  vtkSetMacro( Equalization, double );
  
  // Read/Write from/to file
  std::string ToXMLString( vtkIndent indent );
  void FromXMLElement( vtkXMLDataElement* element );


protected:

  double FilterWidth;
  int OrthogonalOrder;
  int OrthogonalWindow;
  int Derivative;
  int NumCentroids;
  int NumPrinComps;
  double MarkovPseudoScalePi;
  double MarkovPseudoScaleA;
  double MarkovPseudoScaleB;
  double CompletionTime;
  double Equalization;

};

#endif