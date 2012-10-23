
#ifndef VTKRECORDLOG_H
#define VTKRECORDLOG_H

#include "vtkSmartPointer.h"
#include "vtkObject.h"
#include "vtkSlicerWorkflowSegmentationModuleLogicExport.h"

#include <vector>
#include <iostream>


class ValueRecord
{
public:
  double time;
  std::vector<double> values;
};




// Class representing a procedure comprised of tracking records
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_LOGIC_EXPORT
  vtkRecordLog : public vtkObject
{
public:

  static vtkRecordLog *New();

  vtkRecordLog* DeepCopy();

  int Size();

  void AddRecord( ValueRecord newRecord );
  ValueRecord GetRecordAt( int index );
  vtkRecordLog* Trim( int start, int end );

  vtkRecordLog* Derivative( int order = 1 );
  std::vector<double> Integrate();
  std::vector<double> LegendreCoefficient( int order );
  vtkRecordLog* PadStart( int window );
  vtkRecordLog* Concatenate( vtkRecordLog* otherRecLog );
  vtkRecordLog* GaussianFilter( double width );
  vtkRecordLog* OrthogonalTransformation( int window, int order );

private:

  double LegendrePolynomial( double time, int order );	


public:

  vtkRecordLog();
  ~vtkRecordLog();

private:

  std::vector<ValueRecord> records;
  int numRecords;

};

#endif
