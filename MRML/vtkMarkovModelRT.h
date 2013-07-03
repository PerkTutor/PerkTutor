
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

  vtkMarkovModelRT* DeepCopy();

protected:

  // Constructo/destructor
  vtkMarkovModelRT();
  virtual ~vtkMarkovModelRT();

public:

  vtkMarkovRecord* CalculateStateRT( vtkMarkovRecord* element );

protected:

  std::vector<vtkMarkovRecord*> sequence;
  vtkLabelVector* currDelta;
  vtkLabelVector* currPsi;

};

#endif
