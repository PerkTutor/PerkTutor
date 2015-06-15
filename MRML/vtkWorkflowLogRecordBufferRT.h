
#ifndef __vtkWorkflowLogRecordBufferRT_h
#define __vtkWorkflowLogRecordBufferRT_h

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
#include "vtkWorkflowLogRecordBuffer.h"


class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkWorkflowLogRecordBufferRT : public vtkWorkflowLogRecordBuffer
{
public:
  vtkTypeMacro( vtkWorkflowLogRecordBufferRT, vtkObject );

  // Standard MRML methods
  static vtkWorkflowLogRecordBufferRT* New();

protected:

  // Constructo/destructor
  vtkWorkflowLogRecordBufferRT();
  virtual ~vtkWorkflowLogRecordBufferRT();

public:

  // Note: There is no copy function, use the superclass copy function  

  // Methods explicitly for workflow segmentation
  vtkLabelVector* DistancesRT( std::vector< vtkSmartPointer< vtkLabelVector > > labelVectors );

  vtkLabelRecord* DifferentiateRT( int order = 1 );

  vtkLabelRecord* GaussianFilterRT( double width );

  vtkLabelRecord* OrthogonalTransformationRT( int window, int order );

  vtkLabelRecord* TransformPCART( std::vector< vtkSmartPointer< vtkLabelVector > > prinComps, vtkLabelVector* mean );

  vtkLabelRecord* fwdkmeansTransformRT( std::vector< vtkSmartPointer< vtkLabelVector > > centroids );

  vtkMarkovVector* ToMarkovVectorRT();

};

#endif
