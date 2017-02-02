
#ifndef __vtkMRMLWorkflowSequenceNode_h
#define __vtkMRMLWorkflowSequenceNode_h

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
#include "vtkSmartPointer.h"
#include "vtkDoubleArray.h"
#include "vtkMatrix4x4.h"

// VNL includes
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_symmetric_eigensystem.h"



// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkMarkovVector.h"

#include "vtkMRMLSequenceNode.h"
#include "vtkMRMLSequenceBrowserNode.h"
#include "vtkMRMLDoubleArrayNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScene.h"


// This is a sequence node which only takes vtkMRMLDoubleArrayNodes
// But it allows us to do special computations on these nodes
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkMRMLWorkflowSequenceNode : public vtkMRMLSequenceNode
{
public:
  vtkTypeMacro( vtkMRMLWorkflowSequenceNode, vtkMRMLNode );

  // Standard MRML node methods  
  static vtkMRMLWorkflowSequenceNode* New();  
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "WorkflowSequenceNode"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );

protected:

  // Constructor/desctructor
  vtkMRMLWorkflowSequenceNode();
  virtual ~vtkMRMLWorkflowSequenceNode();
  vtkMRMLWorkflowSequenceNode ( const vtkMRMLWorkflowSequenceNode& ); // Required to prevent linking error
  void operator=( const vtkMRMLWorkflowSequenceNode& ); // Required to prevent linking error
  
  
public:

  // Conversion to/from transform buffer
  void FromTrackedSequenceBrowserNode( vtkMRMLSequenceBrowserNode* newTrackedSequenceBrowserNode, std::string proxyNodeID, std::string messagesProxyNodeID, std::vector< std::string > relevantMessages );

  // Convenience method to get index value as a double
  double GetNthIndexValueAsDouble( int itemNumber );
  int GetNthNumberOfComponents( int itemNumber = 0 );
  vtkDoubleArray* GetNthDoubleArray( int itemNumber = 0 );

  // Methods explicitly for workflow segmentation
  void GetSubsequence( int startItemNumber, int endItemNumber, vtkMRMLWorkflowSequenceNode* subsequence );
  void GetLabelledSubsequence( std::vector< std::string > labels, vtkMRMLWorkflowSequenceNode* subsequence );
  
  void Concatenate( vtkMRMLWorkflowSequenceNode* sequence );
  void ConcatenateValues( vtkMRMLWorkflowSequenceNode* sequence );
  void ConcatenateValues( vtkDoubleArray* doubleArray );
  void PadStart( int window );

  void Mean( vtkDoubleArray* meanArray );

  void Distances( vtkMRMLWorkflowSequenceNode* sequence, vtkDoubleArray* distances );
  void Distances( vtkDoubleArray* testPoints, vtkDoubleArray* distances );
  void ClosestRecords( vtkDoubleArray* testPoint, vtkDoubleArray* closest );


  void Differentiate( int order = 1 );
  void Integrate( vtkDoubleArray* integration );
  

  // These are the main steps in the workflow segmentation method
  void GaussianFilter( double width );

  void LegendreTransformation( int order, vtkDoubleArray* legendreCoefficients );
  void OrthogonalTransformation( int window, int order );

  vnl_matrix< double >* CovarianceMatrix();
  void CalculatePrincipalComponents( int numComp, vtkDoubleArray* prinComps );
  void TransformByPrincipalComponents( vtkDoubleArray* prinComps, vtkDoubleArray* mean );

  void fwdkmeans( int numClusters, vtkDoubleArray* centroids );
  void fwdkmeansTransform( vtkDoubleArray* centroids );

  void AddMarkovModelAttributes();

  enum ArrayType
  {
    QUATERNION_ARRAY = 7,
    MATRIX_ARRAY = 16,
  };
    
  // Convert between linear transforms and double arrays
  static void LinearTransformFromDoubleArray( vtkMRMLLinearTransformNode* transformNode, vtkMRMLDoubleArrayNode* doubleArrayNode, ArrayType type );
  static void LinearTransformToDoubleArray( vtkMRMLLinearTransformNode* transformNode, vtkMRMLDoubleArrayNode* doubleArrayNode, ArrayType type );


protected:

  static const double STDEV_CUTOFF;

  double LegendrePolynomial( double time, int order );	

  void FindNextCentroid( vtkDoubleArray* centroids, vtkDoubleArray* nextCentroid );
  bool MembershipChanged( std::vector< int > oldMembership, std::vector< int > newMembership );
  bool HasEmptyClusters( std::vector< bool > emptyVector );
  std::vector< bool > FindEmptyClusters( vtkDoubleArray* centroids, std::vector< int > membership );
  std::vector< int > ReassignMembership( vtkDoubleArray* centroids );
  void MoveEmptyClusters( vtkDoubleArray* centroids, std::vector< bool > emptyVector );
  void RecalculateCentroids( std::vector< int > membership, int numClusters, vtkDoubleArray* centroids );

};  

#endif
