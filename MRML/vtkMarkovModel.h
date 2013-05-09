
#ifndef __vtkMarkovModel_h
#define __vtkMarkovModel_h

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
#include "vtkTrackingRecord.h"


class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkMarkovModel : public vtkObject
{
public:
  vtkTypeMacro( vtkMarkovModel, vtkObject );

  // Standard MRML methods
  static vtkMarkovModel* New();

protected:

  // Constructo/destructor
  vtkMarkovModel();
  virtual ~vtkMarkovModel();

  vtkMarkovModel* DeepCopy();

  void SetSize( int numNewStates, int numNewSymbols );
  int GetNumStates();
  int GetNumSymbols();

  void SetA( std::vector<vtkLabelVector*> newA );
  std::vector<vtkLabelVector*> GetA();
  std::vector<vtkLabelVector*> GetZeroA();
  std::vector<vtkLabelVector*> GetLogA();

  void SetB( std::vector<vtkLabelVector*> newB );
  std::vector<vtkLabelVector*> GetB();
  std::vector<vtkLabelVector*> GetZeroB();
  std::vector<vtkLabelVector*> GetLogB();

  void SetPi( vtkLabelVector* newPi );
  vtkLabelVector* GetPi();
  vtkLabelVector* GetZeroPi();
  vtkLabelVector* GetLogPi();

  void InitializeEstimation( int numEstStates, int numEstSymbols );
  void AddEstimationData( std::vector<vtkMarkovRecord*> sequence );
  void AddPseudoData( vtkLabelVector* pseudoPi, std::vector<vtkLabelVector*> pseudoA, std::vector<vtkLabelVector*> pseudoB );
  void EstimateParameters();
  std::vector<vtkMarkovRecord*> CalculateStates( std::vector<vtkMarkovRecord*> sequence );

  std::string ToString();
  void FromString( std::string s );

private:

	void ZeroParameters();
	void NormalizeParameters();

protected:

	std::vector<vtkLabelVector*> A; // State transition matrix
	std::vector<vtkLabelVector*> B; // Observation matrix
	LabelRecord pi;	// Initial state vector

	int numStates;
	int numSymbols;

};

#endif