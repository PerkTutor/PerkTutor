
#include "vtkMarkovModel.h"
#include "vtkObjectFactory.h"
#include "vtkMRMLWorkflowSequenceNode.h"
#include "vtkMRMLWorkflowTrainingNode.h"

#include <string>
#include <sstream>
#include <cmath>


vtkStandardNewMacro( vtkMarkovModel );


// Constructor & Destructor ----------------------------------------------------------

vtkMarkovModel
::vtkMarkovModel()
{
  this->Pi = vtkSmartPointer< vtkDoubleArray >::New();
  this->A = vtkSmartPointer< vtkDoubleArray >::New();
  this->B = vtkSmartPointer< vtkDoubleArray >::New();

  // No need to pre-allocate for vector
  this->GetZeroPi( this->Pi );
  this->GetZeroA( this->A );
  this->GetZeroB( this->B );
}


vtkMarkovModel
::~vtkMarkovModel()
{
  // Smart pointers take care of themselves
  this->StateNames.clear();
  this->SymbolNames.clear();
}


void vtkMarkovModel
::Copy( vtkMarkovModel* otherMarkov )
{
  this->SetStates( otherMarkov->StateNames );
  this->SetSymbols( otherMarkov->SymbolNames );
  
  this->Pi->DeepCopy( otherMarkov->GetPi() );
  this->A->DeepCopy( otherMarkov->GetA() );
  this->B->DeepCopy( otherMarkov->GetB() );
}


// States & Symbols ----------------------------------------------------------------

int vtkMarkovModel
::GetNumStates()
{
  return this->StateNames.size();
}


int vtkMarkovModel
::GetNumSymbols()
{
  return this->SymbolNames.size();
}


void vtkMarkovModel
::SetStates( std::vector< std::string > newStateNames )
{
  this->StateNames = newStateNames;
}


void vtkMarkovModel
::SetStates( int newStates )
{
  // Assume that we newStates is the number of states we want
  std::vector< std::string > stateNameVector;
  for ( int i = 0; i < newStates; i++ )
  {
    std::stringstream statestring;
	  statestring << i;
	  stateNameVector.push_back( statestring.str() );
  }
  this->SetStates( stateNameVector );
}


void vtkMarkovModel
::SetSymbols( std::vector< std::string > newSymbolNames )
{
  this->SymbolNames = newSymbolNames;
}


void vtkMarkovModel
::SetSymbols( int newSymbols )
{
  // Assume that we newSymbols is the number of symbols we want
  std::vector< std::string > symbolNameVector;
  for ( int i = 0; i < newSymbols; i++ )
  {
    std::stringstream symbolstring;
	  symbolstring << i;
	  symbolNameVector.push_back( symbolstring.str() );
  }
  this->SetSymbols( symbolNameVector );
}


void vtkMarkovModel
::AddState( std::string newStateName )
{
  this->StateNames.push_back( newStateName );
}


void vtkMarkovModel
::AddSymbol( std::string newSymbolName )
{
  this->SymbolNames.push_back( newSymbolName );
}


int vtkMarkovModel
::LookupState( std::string newStateName )
{
  for ( int i = 0; i < this->GetNumStates(); i++ )
  {
    if ( newStateName.compare( this->StateNames.at(i) ) == 0 )
	  {
      return i;
	  }
  }
  return -1;
}


int vtkMarkovModel
::LookupSymbol( std::string newSymbolName )
{
  for ( int i = 0; i < this->GetNumSymbols(); i++ )
  {
    if ( newSymbolName.compare( this->SymbolNames.at(i) ) == 0 )
	  {
      return i;
	  }
  }
  return -1;
}


// Parameters --------------------------------------------------------------

void vtkMarkovModel
::SetPi( vtkDoubleArray* newPi )
{
  if ( newPi->GetNumberOfComponents() != this->GetNumStates() || newPi->GetNumberOfTuples() != 1 )
  {
    vtkWarningMacro( "Initial state vector Pi not set. Input dimensions not consistent with number of states and symbols." );
    return;
  }
  this->Pi = newPi;
}

vtkDoubleArray* vtkMarkovModel
::GetPi()
{
  return this->Pi;
}


void vtkMarkovModel
::GetLogPi( vtkDoubleArray* logPi )
{
  logPi->SetNumberOfComponents( this->Pi->GetNumberOfComponents() );
  logPi->SetNumberOfTuples( this->Pi->GetNumberOfTuples() );  

  for ( int i = 0; i < this->Pi->GetNumberOfTuples(); i++ )
  {
	  for ( int j = 0; j < this->Pi->GetNumberOfComponents(); j++ )
	  {
      logPi->SetComponent( i, j, log( this->Pi->GetComponent( i, j ) ) );
	  }
  }
}


void vtkMarkovModel
::GetZeroPi( vtkDoubleArray* zeroPi )
{
  zeroPi->SetNumberOfComponents( this->GetNumStates() );
  zeroPi->SetNumberOfTuples( 1 );
  vtkMRMLWorkflowSequenceNode::FillDoubleArray( zeroPi, 0 );
}


void vtkMarkovModel
::SetA( vtkDoubleArray* newA )
{
  if ( newA->GetNumberOfComponents() != this->GetNumStates() || newA->GetNumberOfTuples() != this->GetNumStates() )
  {
    vtkWarningMacro( "Transition matrix A not set. Input dimensions not consistent with number of states and symbols." );
    return;
  }
  this->A = newA;
}

vtkDoubleArray* vtkMarkovModel
::GetA()
{
  return this->A;
}


void vtkMarkovModel
::GetLogA( vtkDoubleArray* logA )
{
  logA->SetNumberOfComponents( this->A->GetNumberOfComponents() );
  logA->SetNumberOfTuples( this->A->GetNumberOfTuples() );  

  for ( int i = 0; i < this->A->GetNumberOfTuples(); i++ )
  {
	  for ( int j = 0; j < this->A->GetNumberOfComponents(); j++ )
	  {
      logA->SetComponent( i, j, log( this->A->GetComponent( i, j ) ) );
	  }
  }
}


void vtkMarkovModel
::GetZeroA( vtkDoubleArray* zeroA )
{
  zeroA->SetNumberOfComponents( this->GetNumStates() );
  zeroA->SetNumberOfTuples( this->GetNumStates() );
  vtkMRMLWorkflowSequenceNode::FillDoubleArray( zeroA, 0 );
}


void vtkMarkovModel
::SetB( vtkDoubleArray* newB )
{
  if ( newB->GetNumberOfTuples() != this->GetNumStates() || newB->GetNumberOfComponents() != this->GetNumSymbols() )
  {
    vtkWarningMacro( "Observation matrix B not set. Input dimensions not consistent with number of states and symbols." );
    return;
  }
  this->B = newB;
}

vtkDoubleArray* vtkMarkovModel
::GetB()
{
  return this->B;
}


void vtkMarkovModel
::GetLogB( vtkDoubleArray* logB )
{
  logB->SetNumberOfComponents( this->B->GetNumberOfComponents() );
  logB->SetNumberOfTuples( this->B->GetNumberOfTuples() );  

  for ( int i = 0; i < this->B->GetNumberOfTuples(); i++ )
  {
	  for ( int j = 0; j < this->B->GetNumberOfComponents(); j++ )
	  {
      logB->SetComponent( i, j, log( this->B->GetComponent( i, j ) ) );
	  }
  }
}


void vtkMarkovModel
::GetZeroB( vtkDoubleArray* zeroB )
{
  zeroB->SetNumberOfComponents( this->GetNumSymbols() );
  zeroB->SetNumberOfTuples( this->GetNumStates() );
  vtkMRMLWorkflowSequenceNode::FillDoubleArray( zeroB, 0 );
}






// File input and output ----------------------------------------------------

std::string vtkMarkovModel
::ToXMLString( vtkIndent indent )
{

  std::stringstream xmlstring;

  xmlstring << indent << "<MarkovModel>" << std::endl;

  // State names
  xmlstring << indent.GetNextIndent() << "<States";
  xmlstring << " Size=\"" << this->GetNumStates() << "\"";
  xmlstring << " Values=\"";
  for ( int i = 0; i < this->GetNumStates(); i++ )
  {
    xmlstring << this->StateNames.at(i) << " ";
  }
  xmlstring << "\" />" << std::endl;

  // Symbol names
  xmlstring << indent.GetNextIndent() << "<Symbols";
  xmlstring << " Size=\"" << this->GetNumSymbols() << "\"";
  xmlstring << " Values=\"";
  for ( int i = 0; i < this->GetNumSymbols(); i++ )
  {
    xmlstring << this->SymbolNames.at(i) << " ";
  }
  xmlstring << "\" />" << std::endl;

  // Pi
  xmlstring << vtkMRMLWorkflowTrainingNode::DoubleArrayToXMLString( this->Pi, "MarkovPi", indent.GetNextIndent() );

  // A
  xmlstring << vtkMRMLWorkflowTrainingNode::DoubleArrayToXMLString( this->A, "MarkovA", indent.GetNextIndent() );

  // B
  xmlstring << vtkMRMLWorkflowTrainingNode::DoubleArrayToXMLString( this->B, "MarkovB", indent.GetNextIndent() );
  
  xmlstring << indent << "</MarkovModel>" << std::endl;

  return xmlstring.str();
}


void vtkMarkovModel
::FromXMLElement( vtkXMLDataElement* element )
{

  if ( strcmp( element->GetName(), "MarkovModel" ) != 0 )
  {
    return;  // If it's not a "MarkovModel"
  }

  int numElements = element->GetNumberOfNestedElements();

  for ( int i = 0; i < numElements; i++ )
  {
    vtkXMLDataElement* childElement = element->GetNestedElement( i );

	  // Observe that we cannot use the vtkLabelVector methods since they have integers not strings
    if ( strcmp( childElement->GetName(), "States" ) == 0 && childElement->GetAttribute( "Size" ) != NULL && childElement->GetAttribute( "Values" ) != NULL )
	  {
      this->StateNames.clear();
      int size = atoi( childElement->GetAttribute( "Size" ) );
      
	    std::stringstream instring( childElement->GetAttribute( "Values" ) );
	    std::string value;
      for ( int j = 0; j < size; j++ )
	    {
        instring >> value;
		    this->StateNames.push_back( value );
	    }
      
	  }

	  if ( strcmp( childElement->GetName(), "Symbols" ) == 0 && childElement->GetAttribute( "Size" ) != NULL && childElement->GetAttribute( "Values" ) != NULL )
	  {
      this->SymbolNames.clear();
      int size = atoi( childElement->GetAttribute( "Size" ) );
    
	    std::stringstream instring( childElement->GetAttribute( "Values" ) );
	    std::string value;
      for ( int j = 0; j < size; j++ )
	    {
        instring >> value;
		    this->SymbolNames.push_back( value );
	    }
	  }

    if ( strcmp( childElement->GetName(), "Matrix" ) == 0 && childElement->GetAttribute( "Type" ) != NULL && strcmp( childElement->GetAttribute( "Type" ), "MarkovPi" ) == 0 )
	  {
      vtkMRMLWorkflowTrainingNode::DoubleArrayFromXMLElement( childElement, "MarkovPi", this->Pi );
	  }

	  if ( strcmp( childElement->GetName(), "Matrix" ) == 0 && childElement->GetAttribute( "Type" ) != NULL && strcmp( childElement->GetAttribute( "Type" ), "MarkovA" ) == 0 )
	  {
      vtkMRMLWorkflowTrainingNode::DoubleArrayFromXMLElement( childElement, "MarkovA", this->A );
	  }

	  if ( strcmp( childElement->GetName(), "Matrix" ) == 0 && childElement->GetAttribute( "Type" ) != NULL && strcmp( childElement->GetAttribute( "Type" ), "MarkovB" ) == 0 )
	  {
      vtkMRMLWorkflowTrainingNode::DoubleArrayFromXMLElement( childElement, "MarkovB", this->B );
	  }

  }

}




// All columns in the A and B matrices must sum to one
void vtkMarkovModel
::NormalizeParameters()
{
  // The sum over each tuple should be one
  // That way the total initial state probabiliy, state transition, and emissions probablities are one

  double sumPi = 0;

  for ( int j = 0; j < this->GetNumStates(); j++ )
  {
    sumPi += this->Pi->GetComponent( 0, j );
  }

  // Divide by sum to normalize
  for ( int j = 0; j < this->GetNumStates(); j++ )
  {
    this->Pi->SetComponent( 0, j, this->Pi->GetComponent( 0, j ) / sumPi );
  }

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {
	  double sumA = 0;
	  double sumB = 0;
	
	  // Number of symbols and number of states may be different sizes
	  for ( int j = 0; j < this->GetNumStates(); j++ )
	  {
      sumA += this->A->GetComponent( i, j );
	  }

	  for ( int j = 0; j < this->GetNumSymbols(); j++ )
	  {
      sumB += this->B->GetComponent( i, j );
	  }

	  // Divide by the sum to normalize
	  for ( int j = 0; j < this->GetNumStates(); j++ )
	  {
      this->A->SetComponent( i, j, this->A->GetComponent( i, j ) / sumA );
	  }

	  for ( int j = 0; j < this->GetNumSymbols(); j++ )
	  {
      this->B->SetComponent( i, j, this->B->GetComponent( i, j ) / sumB );
	  }
  }

}


void vtkMarkovModel
::InitializeEstimation()
{
  // Assume that the states and symbols have already been set
  this->GetZeroPi( this->Pi );
  this->GetZeroA( this->A );
  this->GetZeroB( this->B );
}


void vtkMarkovModel
::AddEstimationData( vtkMRMLSequenceNode* sequence )
{
  // Add the data from the current sequence
  int prevStateIndex = -1;

  for ( int i = 0; i < sequence->GetNumberOfDataNodes(); i++ )
  {
    vtkMRMLNode* currDataNode = sequence->GetNthDataNode( i );
    if ( currDataNode == NULL )
    {
      continue;
    }

    int stateIndex = this->LookupState( currDataNode->GetAttribute( "MarkovState" ) );
	  int symbolIndex =  this->LookupSymbol( currDataNode->GetAttribute( "MarkovSymbol" ) );
	  if ( stateIndex < 0 || symbolIndex < 0 )
	  {
      continue;
	  }

    if ( i == 0 ) // Initial observation
    {
      this->Pi->SetComponent( 0, stateIndex, this->Pi->GetComponent( 0, stateIndex ) + 1 );
    }
    else if ( prevStateIndex >= 0 ) // Subsequent observations
    {
      this->A->SetComponent( prevStateIndex, stateIndex, this->A->GetComponent( prevStateIndex, stateIndex ) + 1 );
    }

    this->B->SetComponent( stateIndex, symbolIndex, this->B->GetComponent( stateIndex, symbolIndex ) + 1 );

    prevStateIndex = stateIndex;
  }

}


void vtkMarkovModel
::AddPseudoData( vtkDoubleArray* pseudoPi, vtkDoubleArray* pseudoA, vtkDoubleArray* pseudoB )
{
  // We can simply add the pseudo observations to the estimation counts
  // Note that the order of states should be the same
  for ( int i = 0; i < this->GetNumStates(); i++ )
  {   
    this->Pi->SetComponent( 0, i, this->Pi->GetComponent( 0, i ) + pseudoPi->GetComponent( 0, i ) );

    for ( int j = 0; j < this->GetNumStates(); j++ )
	  {
      this->A->SetComponent( i, j, this->A->GetComponent( i, j ) + pseudoA->GetComponent( i, j ) );
	  }
    for ( int j = 0; j < this->GetNumSymbols(); j++ )
	  {
      this->B->SetComponent( i, j, this->B->GetComponent( i, j ) + pseudoB->GetComponent( i, j ) );
	  }
  }

}


void vtkMarkovModel
::EstimateParameters()
{
  // Normalize the parameters so probabilities add to one
  this->NormalizeParameters();
}


void vtkMarkovModel
::CalculateStates( vtkMRMLSequenceNode* sequence )
{
  // Take the log of all the parameters, so we avoid rounding errors
  vtkNew< vtkDoubleArray > logPi;
  this->GetLogPi( logPi.GetPointer() );
  vtkNew< vtkDoubleArray > logA;
  this->GetLogA( logA.GetPointer() );
  vtkNew< vtkDoubleArray > logB;
  this->GetLogB( logB.GetPointer() );

  // Initialize delta and psi using the initial state distributions
  vtkNew< vtkDoubleArray > delta;
  delta->SetNumberOfComponents( this->GetNumStates() );
  delta->SetNumberOfTuples( sequence->GetNumberOfDataNodes() );
  
  vtkNew< vtkDoubleArray > psi;
  psi->SetNumberOfComponents( this->GetNumStates() );
  psi->SetNumberOfTuples( sequence->GetNumberOfDataNodes() );
  

  // Initializing for the first symbol
  vtkMRMLNode* currDataNode = sequence->GetNthDataNode( 0 );
  if( currDataNode == NULL )
  {
    return;
  }
  int symbolIndex = this->LookupSymbol( currDataNode->GetAttribute( "MarkovSymbol" ) );
  if ( symbolIndex < 0 )
  {
    return;
  }

  for ( int j = 0; j < this->GetNumStates(); j++ )
  {
    delta->SetComponent( 0, j, logPi->GetComponent( 0, j ) + logB->GetComponent( j, symbolIndex ) );
  }

  vtkNew< vtkDoubleArray > initialPsi;
  this->GetZeroPi( initialPsi.GetPointer() );
  psi->SetTuple( 0, 0, initialPsi.GetPointer() );

  // Already calculated for i = 0 (initially)
  for ( int i = 1; i < sequence->GetNumberOfDataNodes(); i++ )
  {
    vtkMRMLNode* currDataNode = sequence->GetNthDataNode( i );
    if( currDataNode == NULL )
    {
      return;
    }
    int symbolIndex = this->LookupSymbol( currDataNode->GetAttribute( "MarkovSymbol" ) );
    if ( symbolIndex < 0 )
    {
      return;
    }

    // Iterate over all states, and check which transition would have been the most likely
    for ( int j = 0; j < this->GetNumStates(); j++ )
	  {

	    int maxIndex = 0;
	    double maxProb = - std::numeric_limits< double >::max();

	    for ( int k = 0; k < this->GetNumStates(); k++ )
	    {
        double currProb = delta->GetComponent( i - 1, k ) + logA->GetComponent( k, j );
        if ( currProb > maxProb ) // Note: A[k].get(j) == A[k][j]
		    {
          maxProb = currProb;
		      maxIndex = k;
		    }
	    }

	    // Account for observation probability
	    delta->SetComponent( i, j, maxProb + logB->GetComponent( j, symbolIndex ) );
      psi->SetComponent( i, j, maxIndex );

	  }
  }

  // Calculate end state
  int endState = 0;
  for ( int k = 0; k < this->GetNumStates(); k++ )
  {
    if ( delta->GetComponent( sequence->GetNumberOfDataNodes() - 1, k ) > delta->GetComponent( sequence->GetNumberOfDataNodes() - 1, endState ) )
	  {
      endState = k;
	  }
  }
  vtkMRMLNode* endDataNode = sequence->GetNthDataNode( sequence->GetNumberOfDataNodes() - 1 );
  if( endDataNode == NULL )
  {
    return;
  }
  endDataNode->SetAttribute( "MarkovState", this->StateNames.at( endState ).c_str() );

  // Calculate prior states from previous states
  for ( int i = sequence->GetNumberOfDataNodes() - 2; i <= 0; i-- )
  {
    vtkMRMLNode* currDataNode = sequence->GetNthDataNode( i );
    vtkMRMLNode* prevDataNode = sequence->GetNthDataNode( i + 1 );
    if( currDataNode == NULL )
    {
      return;
    }
    int prevStateIndex = this->LookupState( prevDataNode->GetAttribute( "MarkovState" ) );
    if ( prevStateIndex < 0 )
    {
      return;
    }


    int currState = psi->GetComponent( i + 1, prevStateIndex );
    currDataNode->SetAttribute( "MarkovState", this->StateNames.at( currState ).c_str() );
  }

  // The states are set in the original nodes in the original sequence
}