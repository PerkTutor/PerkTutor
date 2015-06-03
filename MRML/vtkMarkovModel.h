
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
#include "vtkLabelRecord.h"
#include "vtkMarkovVector.h"


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

public:

  //
  void Copy( vtkMarkovModel* otherMarkov );

  void SetStates( std::vector<std::string> newStateNames );
  void SetStates( int newStates );
  void SetSymbols( std::vector<std::string> newSymbolNames );
  void SetSymbols( int newSymbols );

  void AddState( std::string newStateName );
  void AddSymbol( std::string newSymbolName );

  int LookupState( std::string newStateName );
  int LookupSymbol( std::string newSymbolName );

  int GetNumStates();
  int GetNumSymbols();

  void SetA( std::vector< vtkLabelVector* > newA );
  std::vector< vtkLabelVector* > GetA();
  std::vector< vtkLabelVector* > GetZeroA();
  std::vector< vtkLabelVector* > GetLogA();

  void SetB( std::vector< vtkLabelVector* > newB );
  std::vector< vtkLabelVector* > GetB();
  std::vector< vtkLabelVector* > GetZeroB();
  std::vector< vtkLabelVector* > GetLogB();

  void SetPi( vtkLabelVector* newPi );
  vtkLabelVector* GetPi();
  vtkLabelVector* GetZeroPi();
  vtkLabelVector* GetLogPi();

  void InitializeEstimation();
  void AddEstimationData( std::vector<vtkMarkovRecord*> sequence );
  void AddPseudoData( vtkLabelVector* pseudoPi, std::vector<vtkLabelVector*> pseudoA, std::vector<vtkLabelVector*> pseudoB );
  void EstimateParameters();
  void CalculateStates( std::vector< vtkMarkovRecord* > sequence );

  std::string ToXMLString( vtkIndent indent );
  void FromXMLElement( vtkXMLDataElement* element );

private:

  void ZeroParameters();
  void NormalizeParameters();

protected:

  std::vector< vtkSmartPointer< vtkLabelVector > > A; // State transition matrix
  std::vector< vtkSmartPointer< vtkLabelVector > > B; // Observation matrix
  vtkSmartPointer< vtkLabelVector > Pi;	// Initial state vector

  std::vector< std::string > StateNames;
  std::vector< std::string > SymbolNames;

};

#endif