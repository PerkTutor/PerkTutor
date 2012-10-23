
#ifndef VTKPROCEDURE_H
#define VTKPROCEDURE_H

#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"
#include "vtkObject.h"
#include "vtkSlicerWorkflowSegmentationModuleLogicExport.h"
#include "vtkTrackingRecord.h"

#include <vector>
#include <iostream>



// Class representing a procedure comprised of tracking records
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_LOGIC_EXPORT
  vtkProcedure : public vtkObject
{
public:

  static vtkProcedure *New();

  vtkProcedure* DeepCopy();

  void AddTrackingRecord( vtkTrackingRecord* rec );

  int Size();


  vtkProcedure();
  ~vtkProcedure();

private:

  std::vector< vtkTrackingRecord* > records;
  int numRecords;

};

#endif
