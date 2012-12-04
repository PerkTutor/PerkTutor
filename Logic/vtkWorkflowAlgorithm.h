
#ifndef VTKWORKFLOWALGORITHM_H
#define VTKWORKFLOWALGORITHM_H

#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"
#include "vtkObject.h"
#include "vtkRecordLog.h"
#include "vtkMarkovModel.h"
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

  //vtkWorkflowAlgorithm* DeepCopy();

  // Set the associated MRML node
  void setMRMLNode( vtkMRMLWorkflowSegmentationNode* MRMLNode );
  void GetInputParamtersFromMRMLNode();
  void GetTrainingParametersFromMRMLNode();

  // Read training procedures from file
  void ReadAllProcedures( std::vector<std::string> fileNames );
  void ReadProcedure( std::string fileName );

  // Training and testing phases
  void train();
  
  std::vector<double> CalculateTaskProportions();

  vtkWorkflowAlgorithm();
  ~vtkWorkflowAlgorithm();

private:

  static const int TRACKINGRECORD_SIZE = 7;

  // The associated MRML node
  vtkMRMLWorkflowSegmentationNode* MRMLNode;

  // List of procedures for training
  std::vector<vtkRecordLog*> procedures;

  // All the input parameters
  int NumTasks;
  double FilterWidth;
  int OrthogonalOrder;
  int OrthogonalWindow;
  int Derivative;
  int NumCentroids;
  int NumPrinComps;
  double MarkovPseudoScalePi, MarkovPseudoScaleA, MarkovPseudoScaleB;

  // All the training parameters
  std::vector<LabelRecord> PrinComps; // Size: NumPrinComps, ( Orthogonal Order + 1 ) * 7 * numTools
  std::vector<LabelRecord> Centroids; // Size: NumCentroids, NumPrinComps
  vtkMarkovModel* Markov; // Size: NumTasks, NumCentroids

};

#endif
