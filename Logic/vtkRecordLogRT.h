
#ifndef VTKRECORDLOGRT_H
#define VTKRECORDLOGRT_H

#include "vtkObject.h"
#include "vtkSlicerWorkflowSegmentationModuleLogicExport.h"
#include "vtkRecordLog.h"
#include "RecordType.h"

#include <vector>
#include <iostream>


// Class representing a procedure comprised of tracking records
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_LOGIC_EXPORT
  vtkRecordLogRT : public vtkRecordLog
{
public:

  static vtkRecordLogRT *New();

  void SetMean( ValueRecord newMean );
  void SetPrinComps( std::vector<LabelRecord> newPrinComps );
  void SetCentroids( std::vector<LabelRecord> newCentroids );

  TimeLabelRecord GetRecordRT();

  LabelRecord DistancesRT( std::vector<ValueRecord> valueRecords );

  TimeLabelRecord DerivativeRT( int order = 1 );

  TimeLabelRecord GaussianFilterRT( double width );

  TimeLabelRecord OrthogonalTransformationRT( int window, int order );

  TimeLabelRecord TransformPCART();

  TimeLabelRecord fwdkmeansTransformRT();

public:

  vtkRecordLogRT();
  ~vtkRecordLogRT();

private:

  ValueRecord mean;
  std::vector<LabelRecord> prinComps;
  std::vector<LabelRecord> centroids;

};

#endif
