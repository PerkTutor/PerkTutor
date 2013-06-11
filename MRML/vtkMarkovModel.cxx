
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
  pi = GetZeroPi();
  A = GetZeroA();
  B = GetZeroB();
}


vtkMarkovModel
::~vtkMarkovModel()
{
  vtkDeleteVector( this->A );
  vtkDeleteVector( this->B );
  vtkDelete( pi );

  this->stateNames.clear();
  this->symbolNames.clear();
}


vtkMarkovModel* vtkMarkovModel
::DeepCopy()
{
  vtkMarkovModel* newMarkovModel = vtkMarkovModel::New();

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {
    newMarkovModel->A.push_back( this->GetA().at(i)->DeepCopy() );
	newMarkovModel->B.push_back( this->GetB().at(i)->DeepCopy() );
  }

  newMarkovModel->pi = vtkDeleteAssign( newMarkovModel->pi, this->pi->DeepCopy() );

  newMarkovModel->SetStates( this->stateNames );
  newMarkovModel->SetSymbols( this->symbolNames );

  return newMarkovModel;
}


// States & Symbols ----------------------------------------------------------------

int vtkMarkovModel
::GetNumStates()
{
  return this->stateNames.size();
}


int vtkMarkovModel
::GetNumSymbols()
{
  return this->symbolNames.size();
}


void vtkMarkovModel
::SetStates( std::vector<std::string> newStateNames )
{
  this->stateNames = newStateNames;
}


void vtkMarkovModel
::SetStates( int newStates )
{
  std::vector<std::string> stateNameVector;
  for ( int i = 0; i < newStates; i++ )
  {
    std::stringstream statestring;
	statestring << i;
	stateNameVector.push_back( statestring.str() );
  }
  this->SetStates( stateNameVector );
}


void vtkMarkovModel
::SetSymbols( std::vector<std::string> newSymbolNames )
{
  this->symbolNames = newSymbolNames;
}


void vtkMarkovModel
::SetSymbols( int newSymbols )
{
  std::vector<std::string> symbolNameVector;
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
  this->stateNames.push_back( newStateName );
}


void vtkMarkovModel
::AddSymbol( std::string newSymbolName )
{
  this->symbolNames.push_back( newSymbolName );
}


int vtkMarkovModel
::LookupState( std::string newStateName )
{
  for ( int i = 0; i < this->GetNumStates(); i++ )
  {
    if ( newStateName.compare( this->stateNames.at(i) ) == 0 )
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
    if ( newSymbolName.compare( this->symbolNames.at(i) ) == 0 )
	{
      return i;
	}
  }
  return -1;
}


// Parameters --------------------------------------------------------------
// Note that the parameters should have labels corresponding to the state names, but this is not enforced


void vtkMarkovModel
::SetA( std::vector<vtkLabelVector*> newA )
{
  vtkDeleteVector( this->A );
  this->A = newA;
}

std::vector<vtkLabelVector*> vtkMarkovModel
::GetA()
{
  return this->A;
}


std::vector<vtkLabelVector*> vtkMarkovModel
::GetLogA()
{
  std::vector<vtkLabelVector*> logA;

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {

    vtkLabelVector* currA = vtkLabelVector::New();

	for ( int j = 0; j < this->GetNumStates(); j++ )
	{
      currA->Add( log( this->A.at(i)->Get(j) ) );
	}

	// A must already exist
	currA->SetLabel( this->stateNames.at(i) );
	logA.push_back( currA );
  }

  return logA;
}


std::vector<vtkLabelVector*> vtkMarkovModel
::GetZeroA()
{
  std::vector<vtkLabelVector*> zeroA;

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {

    vtkLabelVector* currA = vtkLabelVector::New();

	for ( int j = 0; j < this->GetNumStates(); j++ )
	{
      currA->Add( 0.0 );
	}

	//Note that A doesn't necessarily already exist
	currA->SetLabel( this->stateNames.at(i) );
	zeroA.push_back( currA );
  }

  return zeroA;
}


void vtkMarkovModel
::SetB( std::vector<vtkLabelVector*> newB )
{
  vtkDeleteVector( this->B );
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
  std::vector<vtkLabelVector*> logB;

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {

    vtkLabelVector* currB = vtkLabelVector::New();

	for ( int j = 0; j < this->GetNumSymbols(); j++ )
	{
      currB->Add( log( this->B.at(i)->Get(j) ) );
	}

	// B must already exist
    currB->SetLabel( this->stateNames.at(i) );
	logB.push_back( currB );
  }

  return logB;
}


std::vector<vtkLabelVector*> vtkMarkovModel
::GetZeroB()
{
  std::vector<vtkLabelVector*> zeroB;

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {

    vtkLabelVector* currB = vtkLabelVector::New();

	for ( int j = 0; j < this->GetNumSymbols(); j++ )
	{
      currB->Add( 0.0 );
	}

	// Note that B doesn't necessarily alread exist
    currB->SetLabel( this->stateNames.at(i) );
	zeroB.push_back( currB );
  }

  return zeroB;
}


void vtkMarkovModel
::SetPi( vtkLabelVector* newPi )
{
  this->pi->Delete();
  this->pi = newPi;
}

vtkLabelVector* vtkMarkovModel
::GetPi()
{
  return this->pi;
}

vtkLabelVector* vtkMarkovModel
::GetLogPi()
{
  vtkLabelVector* logPi = vtkLabelVector::New();

  for ( int j = 0; j < this->GetNumStates(); j++ )
  {
    logPi->Add( log( this->pi->Get(j) ) );
  }

  logPi->SetLabel( "Pi" );
  return logPi;
}


vtkLabelVector* vtkMarkovModel
::GetZeroPi()
{
  vtkLabelVector* zeroPi = vtkLabelVector::New();

  for ( int j = 0; j < this->GetNumStates(); j++ )
  {
    zeroPi->Add( 0.0 );
  }

  zeroPi->SetLabel( "Pi" );
  return zeroPi;
}



// File input and output ----------------------------------------------------

std::string vtkMarkovModel
::ToXMLString()
{

  std::stringstream xmlstring;

  xmlstring << "    <Parameter Type=\"Markov\" >" << std::endl;

  xmlstring << "      <States";
  xmlstring << " Size=\"" << this->GetNumStates() << "\"";
  xmlstring << " Values=\"";
  for ( int i = 0; i < this->GetNumStates(); i++ )
  {
    xmlstring << this->stateNames.at(i) << " ";
  }
  xmlstring << "\" />" << std::endl;

  xmlstring << "      <Symbols";
  xmlstring << " Size=\"" << this->GetNumSymbols() << "\"";
  xmlstring << " Values=\"";
  for ( int i = 0; i < this->GetNumSymbols(); i++ )
  {
    xmlstring << this->symbolNames.at(i) << " ";
  }
  xmlstring << "\" />" << std::endl;

  xmlstring << this->pi->ToXMLString( "MarkovPi" );

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {
    xmlstring << this->A.at(i)->ToXMLString( "MarkovA" );
  }

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {
    xmlstring << this->B.at(i)->ToXMLString( "MarkovB" );
  }

  xmlstring << "    </Parameter>" << std::endl;

  return xmlstring.str();
}


void vtkMarkovModel
::FromXMLElement( vtkXMLDataElement* element )
{

  if ( strcmp( element->GetName(), "Parameter" ) != 0 || strcmp( element->GetAttribute( "Type" ), "Markov" ) != 0 )
  {
    return;  // If it's not a "log" or is the wrong tool jump to the next.
  }

  int numElements = element->GetNumberOfNestedElements();

  // Set up temporary parameters so that we can push to them
  vtkLabelVector* tempPi;
  std::vector<vtkLabelVector*> tempA;
  std::vector<vtkLabelVector*> tempB;

  for ( int i = 0; i < numElements; i++ )
  {
    vtkXMLDataElement* childElement = element->GetNestedElement( i );

	// Observe that we cannot use the vtkLabelVector methods since they have integers not strings
    if ( strcmp( childElement->GetName(), "States" ) == 0 )
	{
	  std::stringstream instring( childElement->GetAttribute( "Values" ) );
	  std::string value;
      for ( int j = 0; j < atoi( childElement->GetAttribute( "Size" ) ); j++ )
	  {
        instring >> value;
		this->stateNames.push_back( value );
	  }
	}

	if ( strcmp( childElement->GetName(), "Symbols" ) == 0 )
	{
	  std::stringstream instring( childElement->GetAttribute( "Values" ) );
	  std::string value;
      for ( int j = 0; j < atoi( childElement->GetAttribute( "Size" ) ); j++ )
	  {
        instring >> value;
		this->symbolNames.push_back( value );
	  }
	}

	if ( strcmp( childElement->GetName(), "MarkovPi" ) == 0 )
	{
      vtkLabelVector* currPi = vtkLabelVector::New();
      currPi->FromXMLElement( childElement, "MarkovPi" );
	  tempPi = currPi;
	}

	if ( strcmp( childElement->GetName(), "MarkovA" ) == 0 )
	{
      vtkLabelVector* currA = vtkLabelVector::New();
      currA->FromXMLElement( childElement, "MarkovA" );
	  tempA.push_back( currA );
	}

	if ( strcmp( childElement->GetName(), "MarkovB" ) == 0 )
	{
      vtkLabelVector* currB = vtkLabelVector::New();
      currB->FromXMLElement( childElement, "MarkovB" );
	  tempB.push_back( currB );
	}

  }

  this->SetPi( tempPi );
  this->SetA( tempA );
  this->SetB( tempB );

}



void vtkMarkovModel
::NormalizeParameters()
{
  
  vtkLabelVector* tempPi = vtkLabelVector::New();
  std::vector<vtkLabelVector*> tempA;
  std::vector<vtkLabelVector*> tempB;

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {

    vtkLabelVector* currA = vtkLabelVector::New();
	vtkLabelVector* currB = vtkLabelVector::New();
	double sumA = 0;
	double sumB = 0;
	
	// Number of symbols and number of states may be different sizes
	for ( int j = 0; j < this->GetNumStates(); j++ )
	{
      sumA += this->A.at(i)->Get(j);
	}

	for ( int j = 0; j < this->GetNumSymbols(); j++ )
	{
      sumB += this->B.at(i)->Get(j);
	}

	// Divide by the sum to normalize
	for ( int j = 0; j < this->GetNumStates(); j++ )
	{
      currA->Add( this->A.at(i)->Get(j) / sumA );
	}

	for ( int j = 0; j < this->GetNumSymbols(); j++ )
	{
      currB->Add( this->B.at(i)->Get(j) / sumB );
	}

    currA->SetLabel( this->A.at(i)->GetLabel() );    
	currB->SetLabel( this->B.at(i)->GetLabel() );

	tempA.push_back( currA );
	tempB.push_back( currB );

  }

  double sumPi = 0;

  for ( int j = 0; j < this->GetNumStates(); j++ )
  {
    sumPi += this->pi->Get(j);
  }

  // Divide by sum to normalize
  for ( int j = 0; j < this->GetNumStates(); j++ )
  {
    tempPi->Add( this->pi->Get(j) / sumPi );
  }

  tempPi->SetLabel( this->pi->GetLabel() );

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
::AddEstimationData( std::vector<vtkMarkovRecord*> sequence )
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
      this->pi->Crement( currState );
    }
	else
    {
	  // Ensure that a previous state exists
	  int prevState = this->LookupState( sequence.at(i-1)->GetState() );
	  if ( prevState >= 0 )
	  {
        this->A.at( prevState )->Crement( currState );
	  }
    }

    this->B.at( currState )->Crement( currSymbol );
  }

}


void vtkMarkovModel
::AddPseudoData( vtkLabelVector* pseudoPi, std::vector<vtkLabelVector*> pseudoA, std::vector<vtkLabelVector*> pseudoB )
{
  // We can simply add the pseudo observations to the estimation counts
  // Note that the order of states should be the same
  for ( int i = 0; i < this->GetNumStates(); i++ )
  {   
    this->pi->Crement( i, pseudoPi->Get(i) );

    for ( int j = 0; j < this->GetNumStates(); j++ )
	{
      this->A.at(i)->Crement( j, pseudoA.at(i)->Get(j) );
	}
    for ( int j = 0; j < this->GetNumSymbols(); j++ )
	{
      this->B.at(i)->Crement( j, pseudoB.at(i)->Get(j) );
	}
  }
}


void vtkMarkovModel
::EstimateParameters()
{
  // Normalize the parameters so probabilities add to one
  this->NormalizeParameters();
}


std::vector<vtkMarkovRecord*> vtkMarkovModel
::CalculateStates( std::vector<vtkMarkovRecord*> sequence )
{
  // Take the log of all the parameters, so we avoid rounding errors
  vtkLabelVector* logPi = this->GetLogPi();
  std::vector<vtkLabelVector*> logA = this->GetLogA();
  std::vector<vtkLabelVector*> logB = this->GetLogB();

  // Initialize delta and psi using the initial state distributions
  std::vector<vtkLabelVector*> delta, psi;
  vtkLabelVector* currDelta = vtkLabelVector::New();
  vtkLabelVector* currPsi = vtkLabelVector::New();

  for ( int j = 0; j < this->GetNumStates(); j++ )
  {
    currDelta->Add( logPi->Get(j) + logB.at(j)->Get( this->LookupSymbol( sequence.at(0)->GetSymbol() ) ) );
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
	  double maxProb = delta.at(i-1)->Get(0) + this->A.at(0)->Get(j);

	  for ( int k = 0; k < this->GetNumStates(); k++ )
	  {
        if ( delta.at(i-1)->Get(k) + A.at(k)->Get(j) > maxProb ) // Note: A[k].get(j) == A[k][j]
		{
          maxProb = delta.at(i-1)->Get(k) + A.at(k)->Get(j);
		  maxIndex = k;
		}
	  }

	  // Account for observation probability
	  currDelta->Add( maxProb + logB.at(j)->Get( this->LookupState( sequence.at(i)->GetSymbol() ) ) ); 
      currPsi->Add( maxIndex );

	}

	delta.push_back( currDelta );
	psi.push_back( currPsi );

  }

  // Calculate end state
  int endState = 0;
  for ( int k = 0; k < this->GetNumStates(); k++ )
  {
    if ( delta.at(sequence.size()-1)->Get(k) > delta.at(sequence.size() - 1)->Get(endState) )
	{
      endState = k;
	}
  }
  sequence.at(sequence.size()-1)->SetState( this->stateNames.at(endState) );

  // Calculate prior states from previous states
  for ( int i = sequence.size() - 2; i <= 0; i-- )
  {
    int currState = psi.at(i+1)->Get( this->LookupState( sequence.at(i+1)->GetState() ) );
    sequence.at(i)->SetState( this->stateNames.at( currState ) );
  }

  // Delete stuff
  logPi->Delete();
  vtkDeleteVector( logA );
  vtkDeleteVector( logB );

  return sequence;

}