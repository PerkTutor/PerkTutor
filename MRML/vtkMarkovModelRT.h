
#ifndef __vtkMarkovModelRT_h
#define __vtkMarkovModelRT_h

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
vtkMarkovModelRT : public vtkMarkovModel
{
public:
  vtkTypeMacro( vtkMarkovModelRT, vtkObject );

  // Standard MRML methods
  static vtkMarkovModelRT* New();

protected:

  // Constructo/destructor
  vtkMarkovModelRT();
  virtual ~vtkMarkovModelRT();

public:

  //
  void Copy( vtkMarkovModelRT* otherMarkov );

  void CalculateStateRT( vtkMarkovVector* element );

protected:

  std::vector< vtkSmartPointer< vtkMarkovVector > > sequence;
  vtkSmartPointer< vtkLabelVector > currDelta;
  vtkSmartPointer< vtkLabelVector > currPsi;

};

#endif
