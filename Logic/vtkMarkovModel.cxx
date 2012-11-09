
#include "vtkMarkovModel.h"
#include "vtkObjectFactory.h"

#include <string>
#include <sstream>
#include <cmath>


vtkStandardNewMacro( vtkMarkovModel );


vtkMarkovModel
::vtkMarkovModel()
{
  numStates = 0;
  numSymbols = 0;
  pi = GetZeroPi();
  A = GetZeroA();
  B = GetZeroB();
}


vtkMarkovModel
::vtkMarkovModel( int initStates, int initSymbols )
{
  numStates = initStates;
  numSymbols = initSymbols;
  pi = GetZeroPi();
  A = GetZeroA();
  B = GetZeroB();
}


vtkMarkovModel
::~vtkMarkovModel()
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

}


vtkMarkovModel* vtkMarkovModel
::DeepCopy()
{

  vtkMarkovModel* newMarkovModel = vtkMarkovModel::New();

  for ( int i = 0; i < numStates; i++ )
  {

	LabelRecord currA;
	LabelRecord currB;
	
	// Number of symbols and number of states may be different sizes
	for ( int j = 0; j < numStates; j++ )
	{
      currA.add( A[i].get(j) );
	}

	for ( int j = 0; j < numSymbols; j++ )
	{
      currB.add( B[i].get(j) );
	}

	currA.setLabel( A[i].getLabel() );
	currB.setLabel( B[i].getLabel() );

    newMarkovModel->A.push_back( currA );
	newMarkovModel->B.push_back( currB );

  }

  LabelRecord currPi;

  for ( int j = 0; j < numStates; j++ )
  {
    currPi.add( pi.get(j) );
  }

  currPi.setLabel( pi.getLabel() );
  newMarkovModel->pi = currPi;

  return newMarkovModel;

}


int vtkMarkovModel
::GetNumStates()
{
  return numStates;
}


int vtkMarkovModel
::GetNumSymbols()
{
  return numSymbols;
}


void vtkMarkovModel
::SetSize( int numNewStates, int numNewSymbols )
{
  numStates = numNewStates;
  numSymbols = numNewSymbols;
}


std::vector<LabelRecord> vtkMarkovModel
::GetA()
{
  return A;
}


std::vector<LabelRecord> vtkMarkovModel
::GetLogA()
{
  std::vector<LabelRecord> logA;

  for ( int i = 0; i < numStates; i++ )
  {

	LabelRecord currA;

	for ( int j = 0; j < numStates; j++ )
	{
      currA.add( log( A[i].get(j) ) );
	}

	// A must already exists
	currA.setLabel( i );
	logA.push_back( currA );
  }

  return logA;
}


std::vector<LabelRecord> vtkMarkovModel
::GetZeroA()
{
  std::vector<LabelRecord> zeroA;

  for ( int i = 0; i < numStates; i++ )
  {

	LabelRecord currA;

	for ( int j = 0; j < numStates; j++ )
	{
      currA.add( 0.0 );
	}

	//Note that A doesn't necessarily already exist
	currA.setLabel( i );
	zeroA.push_back( currA );
  }

  return zeroA;
}


std::vector<LabelRecord> vtkMarkovModel
::GetB()
{
  return B;
}


std::vector<LabelRecord> vtkMarkovModel
::GetLogB()
{
  std::vector<LabelRecord> logB;

  for ( int i = 0; i < numStates; i++ )
  {

	LabelRecord currB;

	for ( int j = 0; j < numSymbols; j++ )
	{
      currB.add( log( B[i].get(j) ) );
	}

	// B must already exist
    currB.setLabel( i );
	logB.push_back( currB );
  }

  return logB;
}


std::vector<LabelRecord> vtkMarkovModel
::GetZeroB()
{
  std::vector<LabelRecord> zeroB;

  for ( int i = 0; i < numStates; i++ )
  {

	LabelRecord currB;

	for ( int j = 0; j < numSymbols; j++ )
	{
      currB.add( 0.0 );
	}

	// Note that B doesn't necessarily alread exist
    currB.setLabel( i );
	zeroB.push_back( currB );
  }

  return zeroB;
}


LabelRecord vtkMarkovModel
::GetPi()
{
  return pi;
}

LabelRecord vtkMarkovModel
::GetLogPi()
{
  LabelRecord logPi;

  for ( int j = 0; j < numStates; j++ )
  {
    logPi.add( log( pi.get(j) ) );
  }

  logPi.setLabel( 0 );
  return logPi;
}


LabelRecord vtkMarkovModel
::GetZeroPi()
{
  LabelRecord zeroPi;

  for ( int j = 0; j < numStates; j++ )
  {
    zeroPi.add( 0.0 );
  }

  zeroPi.setLabel( 0 );
  return zeroPi;
}



void vtkMarkovModel
::NormalizeParameters()
{
  
  std::vector<LabelRecord> newA;
  std::vector<LabelRecord> newB;

  for ( int i = 0; i < numStates; i++ )
  {

	LabelRecord currA;
	LabelRecord currB;
	double sumA = 0;
	double sumB = 0;
	
	// Number of symbols and number of states may be different sizes
	for ( int j = 0; j < numStates; j++ )
	{
      sumA += A[i].get(j);
	}

	for ( int j = 0; j < numSymbols; j++ )
	{
      sumB += B[i].get(j);
	}

	// Divide by the sum to normalize
	for ( int j = 0; j < numStates; j++ )
	{
      currA.add( A[i].get(j) / sumA );
	}

	for ( int j = 0; j < numSymbols; j++ )
	{
      currB.add( B[i].get(j) / sumB );
	}

    currA.setLabel( A[i].getLabel() );
    newA.push_back( currA );
	currB.setLabel( B[i].getLabel() );
	newB.push_back( currB );

  }

  LabelRecord newPi;
  double sumPi = 0;

  for ( int j = 0; j < numStates; j++ )
  {
    sumPi += pi.get(j);
  }

  // Divide by sum to normalize
  for ( int j = 0; j < numStates; j++ )
  {
    newPi.add( pi.get(j) / sumPi );
  }

  newPi.setLabel( pi.getLabel() );

  A = newA;
  B = newB;
  pi = newPi;

}


void vtkMarkovModel
::InitializeEstimation( int numEstStates, int numEstSymbols )
{
  // Reset the number of states and number of symbols equal to the training set sizes
  // Doesn't make sense to have both values and values in training
  numStates = numEstStates;
  numSymbols = numEstSymbols;

  pi = GetZeroPi();
  A = GetZeroA();
  B = GetZeroB();

}


void vtkMarkovModel
::AddEstimationData( std::vector<MarkovRecord> sequence )
{
  // Add the data from the current sequence
  for ( int j = 0; j < sequence.size(); j++ )
  {
    if ( j == 0 )
    {
      pi.increment( sequence[j].state );
    } else
    {
      A[ sequence[j-1].state ].increment( sequence[j].state );
    }

      B[ sequence[j].state ].increment( sequence[j].symbol );
  }

}


void vtkMarkovModel
::AddPseudoData( LabelRecord pseudoPi, std::vector<LabelRecord> pseudoA, std::vector<LabelRecord> pseudoB )
{
  // We can simply add the pseudo observations to the estimation counts
  for ( int i = 0; i < numStates; i++ )
  {
    for ( int j = 0; j < numStates; j++ )
	{
      A[i].set( j, A[i].get(j) + pseudoA[i].get(j) );
	}
    for ( int j = 0; j < numSymbols; j++ )
	{
      B[i].set( j, B[i].get(j) + pseudoB[i].get(j) );
	}
  }

  for ( int j = 0; j < numStates; j++ )
  {
    pi.set( j, pi.get(j) + pseudoPi.get(j) );
  }

}


void vtkMarkovModel
::EstimateParameters()
{
  // Normalize the parameters so probabilities add to one
  NormalizeParameters();
}


std::vector<MarkovRecord> vtkMarkovModel
::CalculateStates( std::vector<MarkovRecord> sequence )
{
  // Take the log of all the parameters, so we avoid rounding errors
  LabelRecord logPi = GetLogPi();
  std::vector<LabelRecord> logA = GetLogA();
  std::vector<LabelRecord> logB = GetLogB();

  // Initialize delta and psi using the initial state distributions
  std::vector<LabelRecord> delta, psi;
  LabelRecord currDelta, currPsi;

  for ( int j = 0; j < numStates; j++ )
  {
    currDelta.add( logPi.get( j ) + logB[j].get( sequence[0].getSymbol() ) );
  }
  delta.push_back( currDelta );

  currPsi = GetZeroPi();
  psi.push_back( currPsi );

  // Already calculated for i = 0 (initially)
  for ( int i = 1; i < sequence.size(); i++ )
  {
    for ( int j = 0; j < numStates; j++ )
	{

	  int maxIndex = 0;
	  double maxProb = delta[i-1].get(0) + A[0].get(j);

	  for ( int k = 0; k < numStates; k++ )
	  {
        if ( delta[i-1].get(k) + A[k].get(j) > maxProb ) // Note: A[k].get(j) == A[k][j]
		{
          maxProb = delta[i-1].get(k) + A[k].get(j);
		  maxIndex = k;
		}
	  }

	  // Account for observation probability
	  currDelta.add( maxProb + logB[j].get( sequence[i].symbol ) ); 
      currPsi.add( maxIndex );

	}

	delta.push_back( currDelta );
	psi.push_back( currPsi );

  }

  // Calculate end state
  int endState = 0;
  for ( int k = 0; k < numStates; k++ )
  {
    if ( delta[ sequence.size() - 1 ].get( k ) > delta[ sequence.size() - 1 ].get( endState ) )
	{
      endState = k;
	}
  }
  sequence[ sequence.size() - 1 ].setState( endState );

  // Calculate prior states from previous states
  for ( int i = sequence.size() - 2; i <= 0; i-- )
  {
    int currState = psi[i+1].get( sequence[i+1].state );
    sequence[ i ].setState( currState );
  }

  return sequence;

}