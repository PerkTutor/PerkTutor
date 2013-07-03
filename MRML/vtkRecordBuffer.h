
#ifndef __vtkRecordBuffer_h
#define __vtkRecordBuffer_h

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

// VNL includes
#include "vnl/vnl_matrix.h"
#include "vnl/algo/vnl_symmetric_eigensystem.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkTrackingRecord.h"
#include "vtkMarkovRecord.h"


class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkRecordBuffer : public vtkObject
{
public:
  vtkTypeMacro( vtkRecordBuffer, vtkObject );

  // Standard MRML methods
  static vtkRecordBuffer* New();

  vtkRecordBuffer* DeepCopy();

protected:

  // Constructo/destructor
  vtkRecordBuffer();
  virtual ~vtkRecordBuffer();
  
  
public:
  
  // Traditional methods for dealing with buffers
  void Initialize( int newNumRecords, int newRecordSize );
  void AddRecord( vtkLabelRecord* newRecord );
  void SetRecordAt( int index, vtkLabelRecord* newRecord );
  void RemoveRecordAt( int index );

  vtkLabelRecord* GetRecordAt( int index );
  vtkLabelRecord* GetCurrentRecord();
  vtkLabelRecord* GetRecordAtTime( double time );

  int GetNumRecords();

  void Clear();

  std::string GetName();
  void SetName( std::string newName );

  vtkMRMLTransformBufferNode* ToTransformBufferNode();
  void FromTransformBufferNode( vtkMRMLTransformBufferNode* newTransformBufferNode );
  void FromTransformBufferNode( vtkMRMLTransformBufferNode* newTransformBufferNode, std::vector<std::string> relevantMessages );

  std::string ToXMLString();
  void FromXMLElement( vtkXMLDataElement* element );

protected:

  static const int STDEV_CUTOFF = 5;
  std::vector<vtkLabelRecord*> records;
  std::string name; // This buffer should be associated with exactly one tool

public:

  // Methods explicitly for workflow segmentation
  vtkRecordBuffer* Trim( int start, int end );

  vtkLabelVector* Mean();

  std::vector<vtkLabelVector*> Distances( vtkRecordBuffer* otherRecLog );
  std::vector<vtkLabelVector*> Distances( std::vector<vtkLabelVector*> vectors );
  vtkLabelRecord* ClosestRecord( vtkLabelVector* vector );

  vtkRecordBuffer* Derivative( int order = 1 );
  vtkLabelVector* Integrate();
  
  vtkRecordBuffer* PadStart( int window );
  vtkRecordBuffer* Concatenate( vtkRecordBuffer* otherRecLog );
  vtkRecordBuffer* ConcatenateValues( vtkRecordBuffer* otherRecLog );
  vtkRecordBuffer* ConcatenateValues( vtkLabelVector* vector );

  vtkRecordBuffer* GaussianFilter( double width );

  std::vector<vtkLabelVector*> LegendreTransformation( int order );
  vtkRecordBuffer* OrthogonalTransformation( int window, int order );

  vnl_matrix<double>* CovarianceMatrix();
  std::vector<vtkLabelVector*> CalculatePCA( int numComp );
  vtkRecordBuffer* TransformPCA( std::vector<vtkLabelVector*> prinComps, vtkLabelVector* mean );

  std::vector<vtkLabelVector*> fwdkmeans( int numClusters );
  vtkRecordBuffer* fwdkmeansTransform( std::vector<vtkLabelVector*> centroids );

  vtkRecordBuffer* TrimBufferByLabel( std::vector<std::string> labels );
  std::vector<vtkRecordBuffer*> SplitBufferByLabel( std::vector<std::string> labels );
  vtkRecordBuffer* AddCompletion( double completionTime );
  std::vector<vtkMarkovRecord*> ToMarkovRecordVector();

private:

  double LegendrePolynomial( double time, int order );	

  vtkLabelVector* FindNextCentroid( std::vector<vtkLabelVector*> centroids );
  bool MembershipChanged( std::vector<int> oldMembership, std::vector<int> newMembership );
  bool HasEmptyClusters( std::vector<bool> emptyVector );
  std::vector<bool> FindEmptyClusters( std::vector<vtkLabelVector*> centroids, std::vector<int> membership );
  std::vector<int> ReassignMembership( std::vector<vtkLabelVector*> centroids );
  std::vector<vtkLabelVector*> MoveEmptyClusters( std::vector<vtkLabelVector*> centroids, std::vector<bool> emptyVector );
  std::vector<vtkLabelVector*> RecalculateCentroids( std::vector<int> membership, int numClusters );

};  

#endif
