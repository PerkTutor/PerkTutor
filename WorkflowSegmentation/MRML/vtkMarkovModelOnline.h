
#ifndef __vtkMarkovModelOnline_h
#define __vtkMarkovModelOnline_h

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

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkMarkovModel.h"


class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkMarkovModelOnline : public vtkMarkovModel
{
public:
  vtkTypeMacro( vtkMarkovModelOnline, vtkObject );

  // Standard MRML methods
  static vtkMarkovModelOnline* New();

protected:

  // Constructo/destructor
  vtkMarkovModelOnline();
  virtual ~vtkMarkovModelOnline();

public:

  //
  void Copy( vtkMarkovModelOnline* otherMarkov );

  void CalculateStateOnline( vtkMRMLNode* node, std::string indexValue );

protected:

  vtkSmartPointer< vtkMRMLSequenceNode > Sequence;
  vtkSmartPointer< vtkDoubleArray > CurrDelta;
  vtkSmartPointer< vtkDoubleArray > CurrPsi;

};

#endif
