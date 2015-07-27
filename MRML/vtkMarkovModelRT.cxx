
#include "vtkMarkovModelRT.h"
#include "vtkObjectFactory.h"


#include <string>
#include <sstream>
#include <cmath>


vtkStandardNewMacro( vtkMarkovModelRT );


vtkMarkovModelRT
::vtkMarkovModelRT()
{
  this->currDelta = vtkSmartPointer< vtkLabelVector >::New();
  this->currPsi = vtkSmartPointer< vtkLabelVector >::New();
}


vtkMarkovModelRT
::~vtkMarkovModelRT()
{
  this->sequence.clear();
}


void vtkMarkovModelRT
::Copy( vtkMarkovModelRT* otherMarkov )
{
  this->vtkMarkovModel::Copy( otherMarkov );

  // Must update the collected sequence. Psi and Delta will update automatically as we add sequence
  for ( int i = 0; i < otherMarkov->sequence.size(); i++ )
  {
    this->CalculateStateRT( otherMarkov->sequence.at(i) );
  }
}


void vtkMarkovModelRT
::CalculateStateRT( vtkMarkovVector* element )
{

  // Take the log of all the parameters, so we avoid rounding errors
  vtkSmartPointer< vtkLabelVector > logPi = vtkSmartPointer< vtkLabelVector >::New();
  this->GetLogPi( logPi );
  std::vector< vtkSmartPointer< vtkLabelVector > > logA;
  logA = this->GetLogA();
  std::vector< vtkSmartPointer< vtkLabelVector > > logB;
  logB = this->GetLogB();

  // This must both calculate the current state and update pi and delta
  // Case there are no previous elements in the sequence
  if ( this->sequence.size() == 0 )
  {
    this->currDelta = vtkSmartPointer< vtkLabelVector >::New();
    
    for ( int j = 0; j < this->GetNumStates(); j++ )
	  {
      this->currDelta->AddElement( logPi->GetElement( j ) + logB.at(j)->GetElement( this->LookupSymbol( element->GetSymbol() ) ) );
	  }
	  this->currDelta->SetLabel( "Delta" );

    this->GetZeroPi( this->currPsi );
	  this->currPsi->SetLabel( "Psi" );
  }

  // Case there are previous elements in the sequence
  if ( this->sequence.size() != 0 )
  {

	  for ( int j = 0; j < this->GetNumStates(); j++ )
	  {

	    int maxIndex = 0;
	    double maxProb = - std::numeric_limits< double >::max();

	    for ( int k = 0; k < this->GetNumStates(); k++ )
	    {
        double currProb = currDelta->GetElement( k ) + A.at(k)->GetElement( j );
        if ( currProb > maxProb ) // Note: A[k].get(j) == A[k][j]
		    {
          maxProb = currProb;
		      maxIndex = k;
		    }
	    }

	    // Account for observation probability
	    this->currDelta->SetElement( j, maxProb + logB[j]->GetElement( this->LookupSymbol( element->GetSymbol() ) ) ); 
      this->currPsi->SetElement( j, maxIndex );
	  }

  }

  // Calculate end state
  int endState = 0;

  for ( int k = 0; k < this->GetNumStates(); k++ )
  {
    if ( this->currDelta->GetElement( k ) > this->currDelta->GetElement( endState ) )
	  {
      endState = k;
	  }
  }

  // Subsitute the calculated state into the inputted MarkovRecord (since the state originally won't make sense anyway)
  element->SetState( this->StateNames.at( endState ) );
  sequence.push_back( element );

}