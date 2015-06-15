
#ifndef __vtkWorkflowLogRecordBuffer_h
#define __vtkWorkflowLogRecordBuffer_h

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

// VNL includes
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_symmetric_eigensystem.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkLabelRecord.h"
#include "vtkMarkovVector.h"
#include "vtkLogRecordBuffer.h"

#include "vtkMRMLTransformBufferNode.h"


class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkWorkflowLogRecordBuffer : public vtkLogRecordBuffer
{
public:
  vtkTypeMacro( vtkWorkflowLogRecordBuffer, vtkObject );

  // Standard MRML methods
  static vtkWorkflowLogRecordBuffer* New();

protected:

  // Constructo/destructor
  vtkWorkflowLogRecordBuffer();
  virtual ~vtkWorkflowLogRecordBuffer();
  
  
public:

  // Note: Need to modify several functions to use vtkLabelRecords, instead of just vtkLogRecords
  void Copy( vtkWorkflowLogRecordBuffer* otherBuffer );


  // Conversion to/from transform buffer
  vtkMRMLTransformBufferNode* ToTransformBufferNode();
  void FromTransformBufferNode( vtkMRMLTransformBufferNode* newTransformBufferNode, std::string transformName, std::vector< std::string > relevantMessages );

  // Methods explicitly for workflow segmentation
  vtkWorkflowLogRecordBuffer* GetRange( int start, int end );
  vtkWorkflowLogRecordBuffer* GetLabelledRange( std::vector< std::string > labels );

  vtkLabelVector* Mean();

  std::vector< vtkSmartPointer< vtkLabelVector > > Distances( vtkWorkflowLogRecordBuffer* otherRecLog );
  std::vector< vtkSmartPointer< vtkLabelVector > > Distances( std::vector< vtkSmartPointer< vtkLabelVector > > vectors );
  vtkLabelRecord* ClosestRecord( vtkLabelVector* vector );

  void Differentiate( int order = 1 );
  vtkLabelVector* Integrate();
  
  void PadStart( int window );
  void Concatenate( vtkWorkflowLogRecordBuffer* otherRecLog );
  void ConcatenateValues( vtkWorkflowLogRecordBuffer* otherRecLog );
  void ConcatenateValues( vtkLabelVector* vector );

  void GaussianFilter( double width );

  std::vector< vtkSmartPointer< vtkLabelVector > > LegendreTransformation( int order );
  void OrthogonalTransformation( int window, int order );

  vnl_matrix< double >* CovarianceMatrix();
  std::vector< vtkSmartPointer< vtkLabelVector > > CalculatePCA( int numComp );
  void TransformPCA( std::vector< vtkSmartPointer< vtkLabelVector > > prinComps, vtkLabelVector* mean );

  std::vector< vtkSmartPointer< vtkLabelVector > > fwdkmeans( int numClusters );
  void fwdkmeansTransform( std::vector< vtkSmartPointer< vtkLabelVector > > centroids );

  std::vector< vtkSmartPointer< vtkMarkovVector > > ToMarkovVectors();
  
  // Read/write to file
  std::string ToXMLString( vtkIndent indent );
  void FromXMLElement( vtkXMLDataElement* element );

protected:

  static const double STDEV_CUTOFF;

  double LegendrePolynomial( double time, int order );	

  vtkLabelVector* FindNextCentroid( std::vector< vtkSmartPointer< vtkLabelVector > > centroids );
  bool MembershipChanged( std::vector< int > oldMembership, std::vector< int > newMembership );
  bool HasEmptyClusters( std::vector< bool > emptyVector );
  std::vector< bool > FindEmptyClusters( std::vector< vtkSmartPointer< vtkLabelVector > > centroids, std::vector< int > membership );
  std::vector< int > ReassignMembership( std::vector< vtkSmartPointer< vtkLabelVector > > centroids );
  std::vector< vtkSmartPointer< vtkLabelVector > > MoveEmptyClusters( std::vector< vtkSmartPointer< vtkLabelVector > > centroids, std::vector< bool > emptyVector );
  std::vector< vtkSmartPointer< vtkLabelVector > > RecalculateCentroids( std::vector< int > membership, int numClusters );

};  

#endif
