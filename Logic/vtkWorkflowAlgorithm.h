
#ifndef VTKWORKFLOWALGORITHM_H
#define VTKWORKFLOWALGORITHM_H

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
#include "vtkSlicerWorkflowSegmentationModuleLogicExport.h"
#include "vtkRecordBufferRT.h"
#include "vtkMarkovModelRT.h"
#include "vtkWorkflowTool.h"

#include "vtkMRMLTransformBufferNode.h"


// Class representing a particular record for tracking data
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_LOGIC_EXPORT
  vtkWorkflowAlgorithm : public vtkObject
{
public:
  vtkTypeMacro(vtkWorkflowAlgorithm,vtkObject);

  static vtkWorkflowAlgorithm *New();

  //vtkWorkflowAlgorithm* DeepCopy();

protected:

  // Constructo/destructor
  vtkWorkflowAlgorithm();
  virtual ~vtkWorkflowAlgorithm();

public:

  // Read training procedures from file
  void AddTrainingBuffer( vtkRecordBuffer* newTrainingProcedure );

  // Training and testing phases
  bool Train();
  void AddCompletionAlgorithm( vtkWorkflowAlgorithm* newCompletionAlgorithm );
  void AddRecord( vtkLabelRecord* newRecord );
  void AddSegmentRecord( vtkLabelRecord* newRecord );

  vtkWorkflowTool* Tool;
  vtkWorkflowTask* CurrentTask;
  vtkWorkflowTask* PrevTask;
  vtkWorkflowTask* DoTask;
  vtkWorkflowTask* DoneTask;

  vtkWorkflowAlgorithm* CompletionAlgorithm;
  std::vector<bool> CompletionVector;

  void SetCompletionVector( std::string currentTask, std::string currentCompletion );
  bool GetCompletionVector( std::string currentTask );
  
  std::vector<double> CalculateTaskProportions();
  std::vector<double> EqualizeTaskProportions();
  std::vector<int> CalculateTaskCentroids();


private:

  // List of procedures for training
  std::vector<vtkRecordBuffer*> TrainingBuffers;

  // The current procedure for real-time segmentation
  vtkRecordBufferRT* BufferRT;
  vtkRecordBufferRT* DerivativeBufferRT;
  vtkRecordBufferRT* FilterBufferRT;
  vtkRecordBufferRT* OrthogonalBufferRT;
  vtkRecordBufferRT* PcaBufferRT;
  vtkRecordBufferRT* CentroidBufferRT;

};

#endif
