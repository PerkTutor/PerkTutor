
#ifndef __vtkTrackingRecord_h
#define __vtkTrackingRecord_h

// Standard Includes
#include <string>
#include <sstream>
#include <vector>
#include <cmath>

// VTK includes
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"
#include "vtkXMLDataElement.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkLabelRecord.h"
#include "vtkMRMLTransformBufferNode.h"

// A tracking record is a label record that uses the ( tx ty tz q1 q2 q3 q4 ) format
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkTrackingRecord : public vtkLabelRecord
{
public:
  vtkTypeMacro( vtkTrackingRecord, vtkObject );

  // Standard MRML methods
  static vtkTrackingRecord* New();

  vtkTrackingRecord* DeepCopy();

protected:

  // Constructo/destructor
  vtkTrackingRecord();
  virtual ~vtkTrackingRecord();

public:

  void FromString( std::string s ); //Overload with size must be 7

  std::string ToMatrixString();
  void FromMatrixString( std::string newMatrixString );

  vtkTransformRecord* ToTransformRecord();
  void FromTransformRecord( vtkTransformRecord* newTransformRecord );

  std::string ToXMLString( std::string name );
  void FromXMLElement( vtkXMLDataElement* element, std::string name );

  static const int TRACKINGRECORD_SIZE = 7;
  static const int MATRIX_SIZE = 16;


};

#endif