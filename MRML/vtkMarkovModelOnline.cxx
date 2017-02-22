
#include "vtkMarkovModelOnline.h"
#include "vtkObjectFactory.h"


#include <string>
#include <sstream>
#include <cmath>


vtkStandardNewMacro( vtkMarkovModelOnline );


vtkMarkovModelOnline
::vtkMarkovModelOnline()
{
  this->Sequence = vtkSmartPointer< vtkMRMLSequenceNode >::New();

  this->CurrDelta = vtkSmartPointer< vtkDoubleArray >::New();
  this->CurrPsi = vtkSmartPointer< vtkDoubleArray >::New();
}


vtkMarkovModelOnline
::~vtkMarkovModelOnline()
{
  // Smart pointers take care of everything
}


void vtkMarkovModelOnline
::Copy( vtkMarkovModelOnline* otherMarkov )
{
  this->vtkMarkovModel::Copy( otherMarkov );

  // Must update the collected sequence. Psi and Delta will update automatically as we add sequence
  for ( int i = 0; i < otherMarkov->Sequence->GetNumberOfDataNodes(); i++ )
  {
    vtkMRMLNode* currDataNode = otherMarkov->Sequence->GetNthDataNode( i );
    std::string currIndexValue = otherMarkov->Sequence->GetNthIndexValue( i );
    this->CalculateStateOnline( currDataNode, currIndexValue );
  }
}


void vtkMarkovModelOnline
::CalculateStateOnline( vtkMRMLNode* node, std::string indexValue )
{
  // Take the log of all the parameters, so we avoid rounding errors
  vtkNew< vtkDoubleArray > logPi;
  this->GetLogPi( logPi.GetPointer() );
  vtkNew< vtkDoubleArray > logA;
  this->GetLogA( logA.GetPointer() );
  vtkNew< vtkDoubleArray > logB;
  this->GetLogB( logB.GetPointer() );

  // Get the symbol index for the node currently being added
  int symbolIndex = this->LookupSymbol( node->GetAttribute( "MarkovSymbol" ) );
  if ( symbolIndex < 0 )
  {
    return;
  }

  // This must both calculate the current state and update pi and delta
  // Case there are no previous elements in the sequence
  if ( this->Sequence->GetNumberOfDataNodes() == 0 )
  {
    this->CurrDelta = vtkSmartPointer< vtkDoubleArray >::New();
    this->CurrDelta->SetNumberOfComponents( this->GetNumStates() );
    this->CurrDelta->SetNumberOfTuples( 1 );    

    for ( int j = 0; j < this->GetNumStates(); j++ )
	  {
      this->CurrDelta->SetComponent( 0, j, logPi->GetComponent( 0, j ) + logB->GetComponent( j, symbolIndex ) );
	  }

    this->GetZeroPi( this->CurrPsi );
  }

  // Case there are previous elements in the sequence
  if ( this->Sequence->GetNumberOfDataNodes() != 0 )
  {
	  for ( int j = 0; j < this->GetNumStates(); j++ )
	  {

	    int maxIndex = 0;
	    double maxProb = - std::numeric_limits< double >::max();

	    for ( int k = 0; k < this->GetNumStates(); k++ )
	    {
        double currProb = this->CurrDelta->GetComponent( 0, k ) + logA->GetComponent( k, j );
        if ( currProb > maxProb ) // Note: A[k].get(j) == A[k][j]
		    {
          maxProb = currProb;
		      maxIndex = k;
		    }
	    }

	    // Account for observation probability
	    this->CurrDelta->SetComponent( 0, j, maxProb + logB->GetComponent( j, symbolIndex ) );
      this->CurrPsi->SetComponent( 0, j, maxIndex );
	  }

  }

  // Calculate end state
  int endState = 0;
  for ( int k = 0; k < this->GetNumStates(); k++ )
  {
    if ( this->CurrDelta->GetComponent( 0, k ) > this->CurrDelta->GetComponent( 0, endState ) )
	  {
      endState = k;
	  }
  }

  // Subsitute the calculated state into the inputted MarkovRecord (since the state originally won't make sense anyway)
  node->SetAttribute( "MarkovState", this->StateNames.at( endState ).c_str() );

  this->Sequence->SetDataNodeAtValue( node, indexValue );
}