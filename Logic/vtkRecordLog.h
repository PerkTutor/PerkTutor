
#ifndef VTKRECORDLOG_H
#define VTKRECORDLOG_H

#include "vtkSmartPointer.h"
#include "vtkObject.h"
#include "vtkSlicerWorkflowSegmentationModuleLogicExport.h"
#include "vnl\vnl_matrix.h"

#include <vector>
#include <iostream>

class ValueRecord
{
public:
  std::vector<double> values;
  void add( double newValue ) { values.push_back( newValue ); };
  void set( int index, double newValue ){ values[index] = newValue; };
  double get( int index ) { return values[index]; };
  int size(){ return values.size(); };
};

class TimeRecord : public ValueRecord
{
public:
  double time;
};

class OrderRecord : public ValueRecord
{
public:
  int order;
};

// Class representing a procedure comprised of tracking records
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_LOGIC_EXPORT
  vtkRecordLog : public vtkObject
{
public:

  static vtkRecordLog *New();

  vtkRecordLog* DeepCopy();

  int Size();

  void AddRecord( TimeRecord newRecord );
  TimeRecord GetRecordAt( int index );
  vtkRecordLog* Trim( int start, int end );

  ValueRecord Mean();

  std::vector<OrderRecord> Distances( vtkRecordLog* otherRecLog );
  std::vector<OrderRecord> Distances( std::vector<ValueRecord> valueRecords );

  vtkRecordLog* Derivative( int order = 1 );
  ValueRecord Integrate();
  
  vtkRecordLog* PadStart( int window );
  vtkRecordLog* Concatenate( vtkRecordLog* otherRecLog );

  vtkRecordLog* GaussianFilter( double width );

  std::vector<OrderRecord> LegendreTransformation( int order );
  vtkRecordLog* OrthogonalTransformation( int window, int order );

  vnl_matrix<double>* CovarianceMatrix();
  std::vector<OrderRecord> CalculatePCA( int numComp );
  vtkRecordLog* TransformPCA( std::vector<OrderRecord> prinComps );

  std::vector<OrderRecord> fwdkmeans( int numClusters, ValueRecord weights );
  

private:

  double LegendrePolynomial( double time, int order );	

  std::vector<OrderRecord> AddNextCentroid( std::vector<OrderRecord> centroids );
  std::vector<int> ReassignMembership( std::vector<OrderRecord> centroids );
  std::vector<OrderRecord> MoveEmptyClusters( std::vector<OrderRecord> centroids, std::vector<int> membership );
  std::vector<OrderRecord> RecalculateCentroids( std::vector<int> membership, int numClusters );


public:

  vtkRecordLog();
  ~vtkRecordLog();

private:

  std::vector<TimeRecord> records;
  int numRecords;
  int recordSize;

};

#endif
