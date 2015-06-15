
#ifndef __vtkMarkovModel_h
#define __vtkMarkovModel_h

// Standard includes
#include <ctime>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>
#include <limits>

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

  void SetA( std::vector< vtkSmartPointer< vtkLabelVector > > newA );
  std::vector< vtkSmartPointer< vtkLabelVector > > GetA();
  std::vector< vtkSmartPointer< vtkLabelVector > > GetZeroA();
  std::vector< vtkSmartPointer< vtkLabelVector > > GetLogA();

  void SetB( std::vector< vtkSmartPointer< vtkLabelVector > > newB );
  std::vector< vtkSmartPointer< vtkLabelVector > > GetB();
  std::vector< vtkSmartPointer< vtkLabelVector > > GetZeroB();
  std::vector< vtkSmartPointer< vtkLabelVector > > GetLogB();

  void SetPi( vtkLabelVector* newPi );
  vtkLabelVector* GetPi();
  vtkLabelVector* GetZeroPi();
  vtkLabelVector* GetLogPi();

  void InitializeEstimation();
  void AddEstimationData( std::vector< vtkSmartPointer< vtkMarkovVector > > sequence );
  void AddPseudoData( vtkLabelVector* pseudoPi, std::vector< vtkSmartPointer< vtkLabelVector > > pseudoA, std::vector< vtkSmartPointer< vtkLabelVector > > pseudoB );
  void EstimateParameters();
  void CalculateStates( std::vector< vtkSmartPointer< vtkMarkovVector > > sequence );

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