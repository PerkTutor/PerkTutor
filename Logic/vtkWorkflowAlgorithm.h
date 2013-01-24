
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
  void setMRMLNode( vtkMRMLWorkflowSegmentationNode* MRMLNode );
  void GetProcedureDefinitionFromMRMLNode();
  void GetInputParamtersFromMRMLNode();
  void GetTrainingParametersFromMRMLNode();

  // Read training procedures from file
  void ReadAllProcedures( std::vector<std::string> fileNames );
  void ReadProcedure( std::string fileName );
  void SegmentProcedure( std::string fileName );

  // Training and testing phases
  void InitializeSegmentationRT();
  bool train();
  void addRecord( TransformRecord t );
  void addSegmentRecord( TransformRecord t );
  void UpdateTask();

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
  vtkMRMLWorkflowSegmentationNode* MRMLNode;

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

  // Procedure definition
  int NumTasks;
  std::vector<std::string> TaskName;
  std::vector<std::string> TaskInstruction;
  std::vector<std::string> TaskNext;

  // All the input parameters
  double FilterWidth;
  int OrthogonalOrder;
  int OrthogonalWindow;
  int Derivative;
  int NumCentroids;
  int NumPrinComps;
  double MarkovPseudoScalePi, MarkovPseudoScaleA, MarkovPseudoScaleB;

  // All the training parameters
  std::vector<LabelRecord> PrinComps; // Size: NumPrinComps, ( Orthogonal Order + 1 ) * 7 * numTools
  LabelRecord Mean; // Size: 7
  std::vector<LabelRecord> Centroids; // Size: NumCentroids, NumPrinComps
  vtkMarkovModel* Markov; // Size: NumTasks, NumCentroids

};

#endif
