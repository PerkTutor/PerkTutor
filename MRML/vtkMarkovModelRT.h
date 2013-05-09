
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
vtkMarkovModelRT : public vtkObject
{
public:
  vtkTypeMacro( vtkMarkovModel, vtkObject );

  // Standard MRML methods
  static vtkMarkovModel* New();

protected:

  // Constructo/destructor
  vtkMarkovModel();
  virtual ~vtkMarkovModel();

  static vtkMarkovModelRT *New();
  vtkTypeMacro(vtkMarkovModelRT,vtkMarkovModel);

  MarkovRecord CalculateStateRT( MarkovRecord element );

protected:

  std::vector<MarkovRecord> sequence;
  LabelRecord currDelta;
  LabelRecord currPsi;

};

#endif
