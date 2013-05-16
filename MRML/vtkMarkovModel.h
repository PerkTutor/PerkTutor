
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
#include "vtkMarkovRecord.h"


class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkMarkovModel : public vtkObject
{
public:
  vtkTypeMacro( vtkMarkovModel, vtkObject );

  // Standard MRML methods
  static vtkMarkovModel* New();

  vtkMarkovModel* DeepCopy();

protected:

  // Constructo/destructor
  vtkMarkovModel();
  virtual ~vtkMarkovModel();

public:

  void SetStates( std::vector<std::string> newStateNames );
  void SetStates( int newStates );
  void SetSymbols( std::vector<std::string> newSymbolsNames );
  void SetSymbols( int newSymbols );

  void AddState( std::string newStateName );
  void AddSymbol( std::string newSymbolName );

  int LookupState( std::string newStateName );
  int LookupSymbol( std::string newSymbolName );

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

  void InitializeEstimation();
  void AddEstimationData( std::vector<vtkMarkovRecord*> sequence );
  void AddPseudoData( vtkLabelVector* pseudoPi, std::vector<vtkLabelVector*> pseudoA, std::vector<vtkLabelVector*> pseudoB );
  void EstimateParameters();
  std::vector<vtkMarkovRecord*> CalculateStates( std::vector<vtkMarkovRecord*> sequence );

  std::string ToXMLString();
  void FromXMLElement( vtkXMLDataElement* element );

private:

  void ZeroParameters();
  void NormalizeParameters();

protected:

  std::vector<vtkLabelVector*> A; // State transition matrix
  std::vector<vtkLabelVector*> B; // Observation matrix
  vtkLabelVector* pi;	// Initial state vector

  std::vector<std::string> stateNames;
  std::vector<std::string> symbolNames;

};

#endif