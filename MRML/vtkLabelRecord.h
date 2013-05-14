
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

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkLabelVector.h"

// A label record is a label vector plus a time attribute
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT 
vtkLabelRecord : public vtkLabelVector
{
public:
  vtkTypeMacro( vtkLabelRecord, vtkObject );

  // Standard MRML methods
  static vtkLabelRecord* New();

  vtkLabelRecord* DeepCopy();

protected:

  // Constructo/destructor
  vtkLabelRecord();
  virtual ~vtkLabelRecord();

private:

  double Time;

public:

  double GetTime();
  int GetSec();
  int GetNSec();
  void SetTime( double newTime );
  void SetTime( int newSec, int newNSec );

  std::string ToXMLString( std::string name );
  void FromXMLElement( vtkXMLDataElement* element, std::string name );

};

#endif