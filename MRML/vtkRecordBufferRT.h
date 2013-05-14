
#ifndef __vtkRecordBufferRT_h
#define __vtkRecordBufferRT_h

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
#include "vtkRecordBuffer.h"


class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkRecordBufferRT : public vtkRecordBuffer
{
public:
  vtkTypeMacro( vtkRecordBufferRT, vtkObject );

  // Standard MRML methods
  static vtkRecordBufferRT* New();

  vtkRecordBufferRT* DeepCopy();

protected:

  // Constructo/destructor
  vtkRecordBufferRT();
  virtual ~vtkRecordBufferRT();

public:

  // Methods explicitly for workflow segmentation
  vtkLabelRecord* GetRecordRT();
  void SetRecordRT( vtkLabelRecord* newRecord );

  vtkLabelVector* DistancesRT( std::vector<vtkLabelVector*> labelVectors );

  vtkLabelRecord* DerivativeRT( int order = 1 );

  vtkLabelRecord* GaussianFilterRT( double width );

  vtkLabelRecord* OrthogonalTransformationRT( int window, int order );

  vtkLabelRecord* TransformPCART( std::vector<vtkLabelVector*> prinComps, vtkLabelVector* mean );

  vtkLabelRecord* fwdkmeansTransformRT( std::vector<vtkLabelVector*> centroids );

  vtkMarkovRecord* ToMarkovRecordRT();

};

#endif
