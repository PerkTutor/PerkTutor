
#ifndef VTKRECORDLOG_H
#define VTKRECORDLOG_H

#include "vtkSmartPointer.h"
#include "vtkObject.h"
#include "vtkSlicerWorkflowSegmentationModuleLogicExport.h"
#include "vnl\vnl_matrix.h"
#include "RecordType.h"

#include <vector>
#include <iostream>


// Class representing a procedure comprised of tracking records
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_LOGIC_EXPORT
  vtkRecordLog : public vtkObject
{
public:

  static vtkRecordLog *New();

  vtkRecordLog* DeepCopy();

  int Size();

  void AddRecord( TimeLabelRecord newRecord );
  TimeLabelRecord GetRecordAt( int index );
  vtkRecordLog* Trim( int start, int end );

  ValueRecord Mean();

  std::vector<LabelRecord> Distances( vtkRecordLog* otherRecLog );
  std::vector<LabelRecord> Distances( std::vector<ValueRecord> valueRecords );

  vtkRecordLog* Derivative( int order = 1 );
  ValueRecord Integrate();
  
  vtkRecordLog* PadStart( int window );
  vtkRecordLog* Concatenate( vtkRecordLog* otherRecLog );
  vtkRecordLog* ConcatenateValues( vtkRecordLog* otherRecLog );

  vtkRecordLog* GaussianFilter( double width );

  std::vector<LabelRecord> LegendreTransformation( int order );
  vtkRecordLog* OrthogonalTransformation( int window, int order );

  vnl_matrix<double>* CovarianceMatrix();
  std::vector<LabelRecord> CalculatePCA( int numComp );
  vtkRecordLog* TransformPCA( std::vector<LabelRecord> prinComps );

  std::vector<LabelRecord> fwdkmeans( int numClusters );
  vtkRecordLog* fwdkmeansTransform( std::vector<LabelRecord> centroids );
  

private:

  double LegendrePolynomial( double time, int order );	

  std::vector<LabelRecord> AddNextCentroid( std::vector<LabelRecord> centroids );
  std::vector<int> ReassignMembership( std::vector<LabelRecord> centroids );
  std::vector<LabelRecord> MoveEmptyClusters( std::vector<LabelRecord> centroids, std::vector<int> membership );
  std::vector<LabelRecord> RecalculateCentroids( std::vector<int> membership, int numClusters );


public:

  vtkRecordLog();
  ~vtkRecordLog();

protected:

  std::vector<TimeLabelRecord> records;
  int numRecords;
  int recordSize;

};

#endif
