
#ifndef __vtkMRMLWorkflowSequenceOnlineNode_h
#define __vtkMRMLWorkflowSequenceOnlineNode_h

// Standard includes
#include <ctime>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

// VTK includes
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"
#include "vtkXMLDataElement.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLWorkflowSequenceNode.h"


class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkMRMLWorkflowSequenceOnlineNode : public vtkMRMLWorkflowSequenceNode
{
public:
  vtkTypeMacro( vtkMRMLWorkflowSequenceOnlineNode, vtkMRMLNode );

  // Standard MRML node methods  
  static vtkMRMLWorkflowSequenceOnlineNode* New();  
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "WorkflowSequenceOnlineNode"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );

protected:

  // Constructor/desctructor
  vtkMRMLWorkflowSequenceOnlineNode();
  virtual ~vtkMRMLWorkflowSequenceOnlineNode();
  vtkMRMLWorkflowSequenceOnlineNode ( const vtkMRMLWorkflowSequenceOnlineNode& ); // Required to prevent linking error
  void operator=( const vtkMRMLWorkflowSequenceOnlineNode& ); // Required to prevent linking error

public:

  // Note: There is no copy function, use the superclass copy function  

  // Methods explicitly for workflow segmentation
  void DistancesOnline( vtkDoubleArray* testPoints, vtkDoubleArray* distances );

  void DifferentiateOnline( int order, vtkDoubleArray* derivative );

  void GaussianFilterOnline( double width, vtkDoubleArray* gauss );

  void OrthogonalTransformationOnline( int window, int order, vtkDoubleArray* orthogonal );

  void TransformByPrincipalComponentsOnline( vtkDoubleArray* prinComps, vtkDoubleArray* meanArray, vtkDoubleArray* transformed );

  void fwdkmeansTransformOnline( vtkDoubleArray* centroids, vtkDoubleArray* cluster );

  void AddMarkovModelAttributesOnline( vtkMRMLNode* node );

};

#endif
