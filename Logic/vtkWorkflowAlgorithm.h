
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
#include "vtkMRMLTransformBufferNode.h"
#include "vtkWorkflowTool.h"


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
  void AddTrainingProcedure( vtkRecordBuffer* newTrainingProcedure );
  void SegmentProcedure( vtkRecordBuffer* newProcedure );

  // Training and testing phases
  void Reset();
  bool Train();
  void AddRecord( vtkLabelRecord* newRecord );
  void AddSegmentRecord( vtkLabelRecord* newRecord );
  void UpdateTask();

  void SetTool( vtkWorkflowTool* newTool );
  vtkWorkflowTool* GetTool();
  vtkWorkflowTool* Tool;

  std::string GetCurrentTask();
  std::string GetCurrentInstruction();
  std::string GetNextTask();
  std::string GetNextInstruction();
  
  std::vector<double> CalculateTaskProportions();
  std::vector<int> CalculateTaskCentroids();


private:

  // List of procedures for training
  std::vector<vtkRecordBuffer*> trainingProcedures;

  // The current procedure for real-time segmentation
  vtkRecordBufferRT* procedureRT;
  vtkRecordBufferRT* derivativeProcedureRT;
  vtkRecordBufferRT* filterProcedureRT;
  vtkRecordBufferRT* orthogonalProcedureRT;
  vtkRecordBufferRT* pcaProcedureRT;
  vtkRecordBufferRT* centroidProcedureRT;

  vtkMarkovModelRT* MarkovRT;

  int IndexToProcess;
  std::string CurrentTask;
  std::string PrevTask;

};

#endif
