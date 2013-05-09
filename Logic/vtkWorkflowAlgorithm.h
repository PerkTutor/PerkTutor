
#ifndef VTKWORKFLOWALGORITHM_H
#define VTKWORKFLOWALGORITHM_H

#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"
#include "vtkObject.h"
#include "vtkRecordLog.h"
#include "vtkRecordLogRT.h"
#include "vtkMarkovModel.h"
#include "vtkMarkovModelRT.h"
#include "vtkMRMLWorkflowSegmentationNode.h"
#include "vtkSlicerWorkflowSegmentationModuleLogicExport.h"
#include "RecordType.h"

#include <vector>
#include <iostream>


// Class representing a particular record for tracking data
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_LOGIC_EXPORT
  vtkWorkflowAlgorithm : public vtkObject
{
public:

  static vtkWorkflowAlgorithm *New();
  vtkTypeMacro(vtkWorkflowAlgorithm,vtkObject);

  //vtkWorkflowAlgorithm* DeepCopy();

  // Set the associated MRML node
  void SetModuleNode( vtkMRMLWorkflowSegmentationNode* newModulNode );
  void GetProcedureDefinitionFromMRMLNode();
  void GetInputParamtersFromMRMLNode();
  void GetTrainingParametersFromMRMLNode();

  // Read training procedures from file
  void ReadAllProcedures( std::vector<std::string> fileNames );
  void ReadProcedure( std::string fileName );
  void SegmentProcedure( std::string fileName );

  // Training and testing phases
  void Reset();
  bool Train();
  void AddRecord( TransformRecord t );
  void AddSegmentRecord( TransformRecord t );
  void UpdateTask();

  void SetToolName( std::string );
  Tool GetTool();

  std::string getCurrentTask();
  std::string getCurrentInstruction();
  std::string getNextTask();
  std::string getNextInstruction();
  int FindTaskIndex( std::string name );
  
  std::vector<double> CalculateTaskProportions();
  std::vector<int> CalculateTaskCentroids();

  vtkWorkflowAlgorithm();
  ~vtkWorkflowAlgorithm();

private:

  static const int TRACKINGRECORD_SIZE = 7;

  // The associated MRML node
  vtkMRMLWorkflowSegmentationNode* ModuleNode;

  // List of procedures for training
  std::vector<vtkRecordLog*> procedures;

  // The current procedure for real-time segmentation
  vtkRecordLogRT* procedureRT;
  vtkRecordLogRT* derivativeProcedureRT;
  vtkRecordLogRT* filterProcedureRT;
  vtkRecordLogRT* orthogonalProcedureRT;
  vtkRecordLogRT* principalProcedureRT;
  vtkRecordLogRT* centroidProcedureRT;

  vtkMarkovModelRT* MarkovRT;

  int indexLastProcessed;
  int currentTask;
  int prevTask;

  // Keep track of the tool this algorithm works for
  std::string toolName;  

};

#endif
