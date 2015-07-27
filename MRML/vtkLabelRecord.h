
#ifndef __vtkLabelRecord_h
#define __vtkLabelRecord_h

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
#include "vtkSetGet.h"
#include "vtkMath.h"

// Perk Tutor includes
#include "vtkLogRecord.h"
#include "vtkTransformRecord.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkLabelVector.h"

// A label record is a label vector plus a time attribute
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkLabelRecord : public vtkLogRecord
{
public:
  vtkTypeMacro( vtkLabelRecord, vtkObject );

  // Standard MRML methods
  static vtkLabelRecord* New();

protected:

  // Constructo/destructor
  vtkLabelRecord();
  virtual ~vtkLabelRecord();

public:

  // Copy
  virtual void Copy( vtkLogRecord* otherRecord );

  enum TrackingRecordType
  {
    QUATERNION_RECORD = 7,
    MATRIX_RECORD = 16,
  };

  // Get/set vector
  vtkLabelVector* GetVector();
  void SetVector( vtkLabelVector* newVector );
    
  // Convert to/from a transform record
  void ToTransformRecord( vtkTransformRecord* transformRecord, TrackingRecordType type );
  void FromTransformRecord( vtkTransformRecord* transformRecord, TrackingRecordType type );

  std::string ToXMLString( vtkIndent indent );
  void FromXMLElement( vtkXMLDataElement* element );
  

  
protected:

  vtkSmartPointer< vtkLabelVector > Vector; 

};

#endif