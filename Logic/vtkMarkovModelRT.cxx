
#include "vtkMarkovModelRT.h"
#include "vtkObjectFactory.h"


#include <string>
#include <sstream>
#include <cmath>


vtkStandardNewMacro( vtkMarkovModelRT );


vtkMarkovModelRT
::vtkMarkovModelRT()
{
  numStates = 0;
  numSymbols = 0;
  pi = GetZeroPi();
  A = GetZeroA();
  B = GetZeroB();
}


vtkMarkovModelRT
::vtkMarkovModelRT( int initStates, int initSymbols )
{
  numStates = initStates;
  numSymbols = initSymbols;
  pi = GetZeroPi();
  A = GetZeroA();
  B = GetZeroB();
}


vtkMarkovModelRT
::~vtkMarkovModelRT()
{

  delete [] &pi;

  // Iterate over all states and delete
  for( int i = 0; i < numStates; i++ )
  {
    delete [] &A[i];
	delete [] &B[i];
  }   

  A.clear();
  B.clear();

  // Iterate the stired sequence
  for ( int i = 0; i < sequence.size(); i++ )
  {
    delete [] &sequence[i];
  }

  sequence.clear();
}


MarkovRecord vtkMarkovModelRT
::CalculateStateRT( MarkovRecord element )
{

  // Take the log of all the parameters, so we avoid rounding errors
  LabelRecord logPi = GetLogPi();
  std::vector<LabelRecord> logA = GetLogA();
  std::vector<LabelRecord> logB = GetLogB();

  // This must both calculate the current state and update pi and delta
  // Case there are no previous elements in the sequence
  if ( sequence.size() == 0 )
  {
    for ( int j = 0; j < numStates; j++ )
	{
      currDelta.add( logPi.get( j ) + logB[j].get( element.getSymbol() ) );
	}
    currPsi = GetZeroPi();

	currDelta.setLabel( 0 );
	currPsi.setLabel( 0 );
  }

  // Case there are previous elements in the sequence
  if ( sequence.size() != 0 )
  {

	for ( int j = 0; j < numStates; j++ )
	{

	  int maxIndex = 0;
	  double maxProb = currDelta.get(0) + A[0].get(j);

	  for ( int k = 0; k < numStates; k++ )
	  {
        if ( currDelta.get(k) + A[k].get(j) > maxProb ) // Note: A[k].get(j) == A[k][j]
		{
          maxProb = currDelta.get(k) + A[k].get(j);
		  maxIndex = k;
		}
	  }

	  // Account for observation probability
	  currDelta.set( j, maxProb + logB[j].get( element.getSymbol() ) ); 
      currPsi.set( j, maxIndex );
	}

  }

  // Calculate end state
  int endState = 0;

  for ( int k = 0; k < numStates; k++ )
  {
    if ( currDelta.get( k ) > currDelta.get( endState ) )
	{
      endState = k;
	}
  }

  // Create a new MarkovRecord with the calculated state
  MarkovRecord currMarkovRecord;
  currMarkovRecord.setState( endState );
  currMarkovRecord.setSymbol( element.getSymbol() );

  sequence.push_back( currMarkovRecord );

  return currMarkovRecord;

}