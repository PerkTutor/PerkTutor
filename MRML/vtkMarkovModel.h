
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
#include "vtkSmartPointer.h"
#include "vtkDoubleArray.h"
#include "vtkNew.h"

// Workflow Segmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkMRMLSequenceNode.h"


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

  void SetPi( vtkDoubleArray* newPi );
  vtkDoubleArray* GetPi();
  void GetZeroPi( vtkDoubleArray* zeroPi );
  void GetLogPi( vtkDoubleArray* logPi );

  void SetA( vtkDoubleArray* newA );
  vtkDoubleArray* GetA();
  void GetZeroA( vtkDoubleArray* zeroA );
  void GetLogA( vtkDoubleArray* logA );

  void SetB( vtkDoubleArray* newB );
  vtkDoubleArray* GetB();
  void GetZeroB( vtkDoubleArray* zeroB );
  void GetLogB( vtkDoubleArray* logB );

  void InitializeEstimation();
  void AddEstimationData( vtkMRMLSequenceNode* sequence );
  void AddPseudoData( vtkDoubleArray* pseudoPi, vtkDoubleArray* pseudoA, vtkDoubleArray* pseudoB );
  void EstimateParameters();
  void CalculateStates( vtkMRMLSequenceNode* sequence );

  std::string ToXMLString( vtkIndent indent );
  void FromXMLElement( vtkXMLDataElement* element );

  static std::string MarkovMatrixToXMLString( vtkDoubleArray* markovMatrix, std::string name, vtkIndent indent );
  static void MarkovMatrixFromXMLElement( vtkXMLDataElement* element, std::string name, vtkDoubleArray* markovMatrix );  

private:

  void ZeroParameters();
  void NormalizeParameters();

protected:

  vtkSmartPointer< vtkDoubleArray > Pi;	// Initial state vector
  vtkSmartPointer< vtkDoubleArray > A; // State transition matrix
  vtkSmartPointer< vtkDoubleArray > B; // Observation matrix

  // The assumption is that the Pi, A, B arrays are always in the same order as the StateNames and SymbolNames

  std::vector< std::string > StateNames;
  std::vector< std::string > SymbolNames;

};

#endif