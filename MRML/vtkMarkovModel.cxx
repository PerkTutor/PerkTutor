
#include "vtkMarkovModel.h"
#include "vtkObjectFactory.h"

#include <string>
#include <sstream>
#include <cmath>


vtkStandardNewMacro( vtkMarkovModel );


// Constructor & Destructor ----------------------------------------------------------

vtkMarkovModel
::vtkMarkovModel()
{
  this->Pi = GetZeroPi();
  this->A = GetZeroA();
  this->B = GetZeroB();
}


vtkMarkovModel
::~vtkMarkovModel()
{
  this->A.clear();
  this->B.clear();

  this->StateNames.clear();
  this->SymbolNames.clear();
}


void vtkMarkovModel
::Copy( vtkMarkovModel* otherMarkov )
{
  this->SetStates( otherMarkov->StateNames );
  this->SetSymbols( otherMarkov->SymbolNames );
  
  this->A.clear();
  this->B.clear();
  for ( int i = 0; i < otherMarkov->GetNumStates(); i++ )
  {
    vtkSmartPointer< vtkLabelVector > currA = vtkSmartPointer< vtkLabelVector >::New();
    currA->Copy( otherMarkov->GetA().at( i ) );
    this->A.push_back( currA );
    
    vtkSmartPointer< vtkLabelVector > currB = vtkSmartPointer< vtkLabelVector >::New();
    currB->Copy( otherMarkov->GetB().at( i ) );
    this->B.push_back( currB );
  }

  this->Pi->Copy( otherMarkov->GetPi() );
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
// Note that the parameters should have labels corresponding to the state names, but this is not enforced


void vtkMarkovModel
::SetA( std::vector< vtkLabelVector* > newA )
{
  this->A.clear();
  this->A = newA;
}

std::vector< vtkLabelVector* > vtkMarkovModel
::GetA()
{
  return this->A;
}


std::vector< vtkLabelVector* > vtkMarkovModel
::GetLogA()
{
  std::vector< vtkSmartPointer< vtkLabelVector > > logA;

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {

    vtkSmartPointer< vtkLabelVector > currLogA = vtkSmartPointer< vtkLabelVector >::New();

	  for ( int j = 0; j < this->GetNumStates(); j++ )
	  {
      currLogA->AddElement( log( this->A.at(i)->GetElement( j ) ) );
	  }

	  // A must already exist
	  currLogA->SetLabel( this->StateNames.at(i) );
	  logA.push_back( currLogA );
  }

  return logA;
}


std::vector< vtkLabelVector > vtkMarkovModel
::GetZeroA()
{
  std::vector< vtkSmartPointer< vtkLabelVector > > zeroA;

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {

    vtkSmartPointer< vtkLabelVector > currZeroA = vtkSmartPointer< vtkLabelVector >::New();

	  for ( int j = 0; j < this->GetNumStates(); j++ )
	  {
      currZeroA->AddElement( 0.0 );
	  }

	  // Note that A doesn't necessarily already exist
	  currZeroA->SetLabel( this->StateNames.at(i) );
	  zeroA.push_back( currZeroA );
  }

  return zeroA;
}


void vtkMarkovModel
::SetB( std::vector< vtkLabelVector* > newB )
{
  this->B.clear();
  this->B = newB;
}

std::vector<vtkLabelVector*> vtkMarkovModel
::GetB()
{
  return this->B;
}


std::vector<vtkLabelVector*> vtkMarkovModel
::GetLogB()
{
  std::vector< vtkSmartPointer< vtkLabelVector > > logB;

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {

    vtkSmartPointer< vtkLabelVector > currLogB = vtkSmartPointer< vtkLabelVector >::New();

	  for ( int j = 0; j < this->GetNumSymbols(); j++ )
	  {
      currLogB->AddElement( log( this->B.at(i)->GetElement( j ) ) );
	  }

	  // B must already exist
    currLogB->SetLabel( this->StateNames.at(i) );
	  logB.push_back( currLogB );
  }

  return logB;
}


std::vector<vtkLabelVector*> vtkMarkovModel
::GetZeroB()
{
  std::vector< vtkSmartPointer< vtkLabelVector > > zeroB;

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {

    vktSmartPointer< vtkLabelVector > currZeroB = vtkSmartPointer< vtkLabelVector >::New();

	  for ( int j = 0; j < this->GetNumSymbols(); j++ )
	  {
      currZeroB->AddElement( 0.0 );
	  }

	  // Note that B doesn't necessarily alread exist
    currZeroB->SetLabel( this->StateNames.at(i) );
	  zeroB.push_back( currZeroB );
  }

  return zeroB;
}


void vtkMarkovModel
::SetPi( vtkLabelVector* newPi )
{
  this->Pi = newPi;
}

vtkLabelVector* vtkMarkovModel
::GetPi()
{
  return this->Pi;
}

vtkLabelVector* vtkMarkovModel
::GetLogPi()
{
  vtkSmartPointer< vtkLabelVector > logPi = vtkSmartPointer< vtkLabelVector >::New();

  // Note: Pi must already exist
  for ( int j = 0; j < this->GetNumStates(); j++ )
  {
    logPi->AddElement( log( this->Pi->GetElement( j ) ) );
  }

  logPi->SetLabel( "Pi" );
  return logPi;
}


vtkLabelVector* vtkMarkovModel
::GetZeroPi()
{
  vtkSmartPointer< vtkLabelVector > zeroPi = vtkSmartPointer< vtkLabelVector >::New();

  for ( int j = 0; j < this->GetNumStates(); j++ )
  {
    zeroPi->AddElement( 0.0 );
  }

  zeroPi->SetLabel( "Pi" );
  return zeroPi;
}



// File input and output ----------------------------------------------------

std::string vtkMarkovModel
::ToXMLString( vtkIndent indent )
{

  std::stringstream xmlstring;

  xmlstring << indent << "<MarkovModel Type=\"Markov\" >" << std::endl;

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
  xmlstring << vtkLabelVector::VectorsToXMLString( this->Pi, "MarkovPi", indent.GetNextIndent() );

  // A
  xmlstring << vtkLabelVector::VectorsToXMLString( this->A, "MarkovA", indent.GetNextIndent() );

  // B
  xmlstring << vtkLabelVector::VectorsToXMLString( this->B, "MarkovB", indent.GetNextIndent() );
  
  xmlstring << indent << "</MarkovModel>" << std::endl;

  return xmlstring.str();
}


void vtkMarkovModel
::FromXMLElement( vtkXMLDataElement* element )
{

  if ( strcmp( element->GetName(), "MarkovModel" ) != 0 || strcmp( element->GetAttribute( "Type" ), "Markov" ) != 0 )
  {
    return;  // If it's not a "MarkovModel"
  }

  int numElements = element->GetNumberOfNestedElements();

  // Set up temporary parameters so that we can push to them
  vtkSmartPointer< vtkLabelVector > tempPi;
  std::vector< vtkSmartPointer< vtkLabelVector > > tempA;
  std::vector< vtkSmartPointer< vtkLabelVector > > tempB;

  for ( int i = 0; i < numElements; i++ )
  {
    vtkXMLDataElement* childElement = element->GetNestedElement( i );

	  // Observe that we cannot use the vtkLabelVector methods since they have integers not strings
    if ( strcmp( childElement->GetName(), "States" ) == 0 )
	  {
      this->StateNames.clear();
      int size = atoi( childElement->GetAttribute( "Size" ) )
      
	    std::stringstream instring( childElement->GetAttribute( "Values" ) );
	    std::string value;
      for ( int j = 0; j < size; j++ )
	    {
        instring >> value;
		    this->StateNames.push_back( value );
	    }
      
	  }

	  if ( strcmp( childElement->GetName(), "Symbols" ) == 0 )
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

	  if ( strcmp( childElement->GetName(), "MarkovPi" ) == 0 )
	  {
      tempPi = vtkSmartPointer< vtkLabelVector >::New();
      tempPi->FromXMLElement( childElement, "MarkovPi" );
	  }

	  if ( strcmp( childElement->GetName(), "MarkovA" ) == 0 )
	  {
      tempA = vtkLabelVector::VectorsFromXMLElement( childElement, "MarkovA" );
	  }

	  if ( strcmp( childElement->GetName(), "MarkovB" ) == 0 )
	  {
      tempB = vtkLabelVector::VectorsFromXMLElement( childElement, "MarkovB" );
	  }

  }

  this->SetPi( tempPi );
  this->SetA( tempA );
  this->SetB( tempB );

}




// All columns in the A and B matrices must sum to one
void vtkMarkovModel
::NormalizeParameters()
{
  
  vtkSmartPointer< vtkLabelVector > tempPi = vtkSmartPointer< vtkLabelVector >::New();
  std::vector< vtkSmartPointer< vtkLabelVector > > tempA;
  std::vector< vtkSmartPointer< vtkLabelVector > > tempB;

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {

    vtkSmartPointer< vtkLabelVector > currA = vtkSmartPointer< vtkLabelVector >::New();
	  vtkSmartPointer< vtkLabelVector > currB = vtkSmartPointer< vtkLabelVector >::New();
	  double sumA = 0;
	  double sumB = 0;
	
	  // Number of symbols and number of states may be different sizes
	  for ( int j = 0; j < this->GetNumStates(); j++ )
	  {
      sumA += this->A.at(i)->GetElement( j );
	  }

	  for ( int j = 0; j < this->GetNumSymbols(); j++ )
	  {
      sumB += this->B.at(i)->GetElement( j );
	  }

	  // Divide by the sum to normalize
	  for ( int j = 0; j < this->GetNumStates(); j++ )
	  {
      currA->AddElement( this->A.at(i)->GetElement( j ) / sumA );
	  }

	  for ( int j = 0; j < this->GetNumSymbols(); j++ )
	  {
      currB->AddElement( this->B.at(i)->GetElement( j ) / sumB );
	  }

    currA->SetLabel( this->A.at(i)->GetLabel() );    
	  currB->SetLabel( this->B.at(i)->GetLabel() );

	  tempA.push_back( currA );
	  tempB.push_back( currB );

  }

  double sumPi = 0;

  for ( int j = 0; j < this->GetNumStates(); j++ )
  {
    sumPi += this->Pi->GetElement( j );
  }

  // Divide by sum to normalize
  for ( int j = 0; j < this->GetNumStates(); j++ )
  {
    tempPi->AddElement( this->Pi->GetElement( j ) / sumPi );
  }

  tempPi->SetLabel( this->Pi->GetLabel() );

  this->SetPi( tempPi );
  this->SetA( tempA );
  this->SetB( tempB );

}


void vtkMarkovModel
::InitializeEstimation()
{
  // Reset the number of states and number of symbols equal to the training set sizes
  // Doesn't make sense to have both values and values in training
  // Assume that the states and symbols have already been set

  this->SetPi( this->GetZeroPi() );
  this->SetA( this->GetZeroA() );
  this->SetB( this->GetZeroB() );
}


void vtkMarkovModel
::AddEstimationData( std::vector< vtkMarkovVector* > sequence )
{
  // Add the data from the current sequence
  for ( int i = 0; i < sequence.size(); i++ )
  {
    int currState = this->LookupState( sequence.at(i)->GetState() );
	  int currSymbol =  this->LookupSymbol( sequence.at(i)->GetSymbol() );

	  if ( currState < 0 || currSymbol < 0 )
	  {
      continue;
	  }

    if ( i == 0 )
    {
      this->Pi->IncrementElement( currState );
    }
	  else
    {
	    // Ensure that a previous state exists
	    int prevState = this->LookupState( sequence.at( i - 1 )->GetState() );
	    if ( prevState >= 0 )
	    {
        this->A.at( prevState )->IncrementElement( currState );
	    }
    }

    this->B.at( currState )->IncrementElement( currSymbol );
  }

}


void vtkMarkovModel
::AddPseudoData( vtkLabelVector* pseudoPi, std::vector< vtkLabelVector* > pseudoA, std::vector< vtkLabelVector* > pseudoB )
{
  // We can simply add the pseudo observations to the estimation counts
  // Note that the order of states should be the same
  for ( int i = 0; i < this->GetNumStates(); i++ )
  {   
    this->Pi->IncrementElement( i, pseudoPi->GetElement( i ) );

    for ( int j = 0; j < this->GetNumStates(); j++ )
	  {
      this->A.at(i)->IncrementElement( j, pseudoA.at(i)->GetElement( j ) );
	  }
    for ( int j = 0; j < this->GetNumSymbols(); j++ )
	  {
      this->B.at(i)->IncrementElement( j, pseudoB.at(i)->GetElement( j ) );
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
::CalculateStates( std::vector< vtkMarkovRecord* > sequence )
{
  // Take the log of all the parameters, so we avoid rounding errors
  vtkSmartPointer< vtkLabelVector > logPi = this->GetLogPi();
  std::vector< vtkSmartPointer< vtkLabelVector > > logA = this->GetLogA();
  std::vector< vtkSmartPointer< vtkLabelVector > > logB = this->GetLogB();

  // Initialize delta and psi using the initial state distributions
  std::vector< vtkSmartPointer< vtkLabelVector > > delta, psi;
  vtkSmartPointer< vtkLabelVector > currDelta = vtkSmartPointer< vtkLabelVector >::New();
  vtkSmartPointer< vtkLabelVector > currPsi = vtkSmartPointer< vtkLabelVector >::New();

  for ( int j = 0; j < this->GetNumStates(); j++ )
  {
    currDelta->Add( logPi->GetElement(j) + logB.at(j)->GetElement( this->LookupSymbol( sequence.at( 0 )->GetSymbol() ) ) );
  }
  delta.push_back( currDelta );

  currPsi = this->GetZeroPi();
  psi.push_back( currPsi );

  // Already calculated for i = 0 (initially)
  for ( int i = 1; i < sequence.size(); i++ )
  {
    for ( int j = 0; j < this->GetNumStates(); j++ )
	  {

	    int maxIndex = 0;
	    double maxProb = - std::numeric_limits< double >::max();

	    for ( int k = 0; k < this->GetNumStates(); k++ )
	    {
        double currProb = delta.at( i - 1 )->GetElement( k ) + A.at(k)->GetElement( j );
        if ( currProb > maxProb ) // Note: A[k].get(j) == A[k][j]
		    {
          maxProb = currProb;
		      maxIndex = k;
		    }
	    }

	    // Account for observation probability
	    currDelta->Add( maxProb + logB.at(j)->GetElement( this->LookupState( sequence.at(i)->GetSymbol() ) ) ); 
      currPsi->Add( maxIndex );

	  }

	  delta.push_back( currDelta );
	  psi.push_back( currPsi );

  }

  // Calculate end state
  int endState = 0;
  for ( int k = 0; k < this->GetNumStates(); k++ )
  {
    if ( delta.at( sequence.size() - 1 )->GetElement( k ) > delta.at( sequence.size() - 1 )->GetElement( endState ) )
	  {
      endState = k;
	  }
  }
  sequence.at( sequence.size() - 1 )->SetState( this->StateNames.at( endState ) );

  // Calculate prior states from previous states
  for ( int i = sequence.size() - 2; i <= 0; i-- )
  {
    int currState = psi.at( i + 1 )->GetElement( this->LookupState( sequence.at( i + 1 )->GetState() ) );
    sequence.at(i)->SetState( this->StateNames.at( currState ) );
  }

  // The states are set in the original vector of markov vectors
}