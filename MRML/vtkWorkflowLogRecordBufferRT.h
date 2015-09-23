
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
  void DistancesRT( std::vector< vtkSmartPointer< vtkLabelVector > > labelVectors, vtkLabelVector* distanceVector );

  void DifferentiateRT( int order, vtkLabelRecord* diffRecord );

  void GaussianFilterRT( double width, vtkLabelRecord* gaussRecord );

  void OrthogonalTransformationRT( int window, int order, vtkLabelRecord* orthogonalRecord );

  void TransformPCART( std::vector< vtkSmartPointer< vtkLabelVector > > prinComps, vtkLabelVector* mean, vtkLabelRecord* pcaTransformRecord );

  void fwdkmeansTransformRT( std::vector< vtkSmartPointer< vtkLabelVector > > centroids, vtkLabelRecord* fwdkmeansRecord );

  void ToMarkovVectorRT( vtkMarkovVector* markovVector );

};

#endif
