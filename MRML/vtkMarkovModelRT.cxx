
#include "vtkMarkovModelRT.h"
#include "vtkObjectFactory.h"


#include <string>
#include <sstream>
#include <cmath>


vtkStandardNewMacro( vtkMarkovModelRT );


vtkMarkovModelRT
::vtkMarkovModelRT()
{
  this->currDelta = NULL;
  this->currPsi = NULL;
}


vtkMarkovModelRT
::~vtkMarkovModelRT()
{
  vtkDeleteVector( this->sequence );

  if ( this->currPsi != NULL )
  {
    this->currPsi->Delete();
    this->currDelta->Delete();
  }
}


vtkMarkovModelRT* vtkMarkovModelRT
::DeepCopy()
{
  vtkMarkovModelRT* newMarkovModelRT = vtkMarkovModelRT::New();

  for ( int i = 0; i < this->GetNumStates(); i++ )
  {
    newMarkovModelRT->A.push_back( this->GetA().at(i)->DeepCopy() );
	newMarkovModelRT->B.push_back( this->GetB().at(i)->DeepCopy() );
  }

  newMarkovModelRT->pi = vtkDeleteAssign( newMarkovModelRT->pi, this->pi->DeepCopy() );

  newMarkovModelRT->SetStates( this->stateNames );
  newMarkovModelRT->SetSymbols( this->symbolNames );

  // Must update the collecte sequence. Psi and Delta will update automatically as we add sequence
  for ( int i = 0; i < this->sequence.size(); i++ )
  {
    newMarkovModelRT->CalculateStateRT( this->sequence.at(i) );
  }

  return newMarkovModelRT;
}


vtkMarkovRecord* vtkMarkovModelRT
::CalculateStateRT( vtkMarkovRecord* element )
{

  // Take the log of all the parameters, so we avoid rounding errors
  vtkLabelVector* logPi = this->GetLogPi();
  std::vector<vtkLabelVector*> logA = this->GetLogA();
  std::vector<vtkLabelVector*> logB = this->GetLogB();

  // This must both calculate the current state and update pi and delta
  // Case there are no previous elements in the sequence
  if ( this->sequence.size() == 0 )
  {
    this->currDelta = vtkLabelVector::New();
    for ( int j = 0; j < this->GetNumStates(); j++ )
	{
      this->currDelta->Add( logPi->Get(j) + logB.at(j)->Get( this->LookupSymbol( element->GetSymbol() ) ) );
	}
	this->currDelta->SetLabel( "Delta" );

    this->currPsi = this->GetZeroPi();
	this->currPsi->SetLabel( "Psi" );
  }

  // Case there are previous elements in the sequence
  if ( this->sequence.size() != 0 )
  {

	for ( int j = 0; j < this->GetNumStates(); j++ )
	{

	  int maxIndex = 0;
	  double maxProb = this->currDelta->Get(0) + this->A.at(0)->Get(j);

	  for ( int k = 0; k < this->GetNumStates(); k++ )
	  {
        if ( this->currDelta->Get(k) + this->A.at(k)->Get(j) > maxProb ) // Note: A[k].get(j) == A[k][j]
		{
          maxProb = this->currDelta->Get(k) + A.at(k)->Get(j);
		  maxIndex = k;
		}
	  }

	  // Account for observation probability
	  this->currDelta->Set( j, maxProb + logB[j]->Get( this->LookupSymbol( element->GetSymbol() ) ) ); 
      this->currPsi->Set( j, maxIndex );
	}

  }

  // Calculate end state
  int endState = 0;

  for ( int k = 0; k < this->GetNumStates(); k++ )
  {
    if ( this->currDelta->Get( k ) > this->currDelta->Get( endState ) )
	{
      endState = k;
	}
  }

  // Delete stuff
  logPi->Delete();
  vtkDeleteVector( logA );
  vtkDeleteVector( logB );

  // Subsitute the calculated state into the inputted MarkovRecord (since the state originally won't make sense anyway)
  element->SetState( this->stateNames.at(endState) );

  sequence.push_back( element );

  return element;

}