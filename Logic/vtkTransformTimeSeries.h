
#ifndef VTKTRANSFORMTIMESERIES_H
#define VTKTRANSFORMTIMESERIES_H


#include <iostream>
#include <utility>
#include <vector>

#include "vtkObject.h"

#include "vtkSlicerPerkEvaluatorModuleLogicExport.h"

#include "vtkSmartPointer.h"
#include "vtkTransform.h"


/**
 * Class around a vector of pairs (double, vtkTransform).
 */
class VTK_SLICER_PERKEVALUATOR_MODULE_LOGIC_EXPORT
vtkTransformTimeSeries
 : public vtkObject
{

public:
  
  static vtkTransformTimeSeries *New();
  void PrintSelf( ostream& os, vtkIndent indent );
  vtkTypeMacro( vtkTransformTimeSeries, vtkObject );
  
  void Copy( vtkTransformTimeSeries* from );
  
  int GetNumberOfRecords() const;
  double GetTimeAtIndex( int index ) const;
  vtkTransform* GetTransformAtIndex( int index ) const;
  vtkSmartPointer< vtkMatrix4x4 > GetMatrixAtIndex( int index );
  
  void AddRecord( double time, vtkTransform* transform );
  void Clear();
  
  
  vtkGetMacro( MinTime, double );
  vtkGetMacro( MaxTime, double );
  
  void SetToolName( std::string name ) {
    this->ToolName = name;
  }
  
  std::string GetToolName() {
    return this->ToolName;
  }
  
  
  //BTX
  typedef std::vector< std::pair< double, vtkTransform* > > DataType;
  typedef DataType::iterator DataIteratorType;
  //ETX
  

protected:

  vtkTransformTimeSeries();
  ~vtkTransformTimeSeries();


private:
  
  vtkTransformTimeSeries( const vtkTransformTimeSeries& ); // Not implemented.
  void operator=( const vtkTransformTimeSeries& ); // Not implemented.
  
  
  DataType Data; // std::vector
  double MinTime;
  double MaxTime;
  std::string ToolName;
  
};


#endif
