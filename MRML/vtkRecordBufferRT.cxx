
#include "vtkRecordBufferRT.h"

vtkStandardNewMacro( vtkRecordBufferRT );

vtkRecordBufferRT
::vtkRecordBufferRT()
{
  this->name = "";
}


vtkRecordBufferRT
::~vtkRecordBufferRT()
{
}


vtkRecordBufferRT* vtkRecordBufferRT
::DeepCopy()
{
  vtkRecordBufferRT* newRecordBufferRT = vtkRecordBufferRT::New();

  newRecordBufferRT->SetName( this->GetName() );
  newRecordBufferRT->records = vtkDeepCopyVector( this->records );

  return newRecordBufferRT;
}


vtkLabelRecord* vtkRecordBufferRT
::GetRecordRT()
{
  return GetRecordAt( this->records.size() - 1 );
}


void vtkRecordBufferRT
::SetRecordRT( vtkLabelRecord* newRecord )
{
  return SetRecordAt( this->GetNumRecords() - 1, newRecord );
}


vtkLabelVector* vtkRecordBufferRT
::DistancesRT( std::vector<vtkLabelVector*> vectors )
{
  // Create a new order record
  vtkLabelVector* distRecord = vtkLabelVector::New();
  distRecord->Initialize( vectors.size(), 0.0 );

  double currSum;

  for ( int j = 0; j < vectors.size(); j++ )
  {
      
    // First, ensure that the records are the same size
    if ( this->GetRecordAt(0)->Size() != vectors[j]->Size() )
    {
      return distRecord;
    }

    // Initialize the sum to zero
    currSum = 0.0;

    for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
    {
      currSum = currSum + ( this->GetRecordRT()->Get(d) - vectors[j]->Get(d) ) * ( this->GetRecordRT()->Get(d) - vectors[j]->Get(d) );
	}
	// Add to the current order record
	distRecord->Set( j, currSum );
  }

  // Add the current order record to the vector
  distRecord->SetLabel( 0 );

  return distRecord;

}


vtkLabelRecord* vtkRecordBufferRT
::DerivativeRT( int order )
{
  // To calculate a derivative of arbitrary order, we need arbitrarily many time stamps
  // Just calculate zeroth order first order derivative here, otherwise use other method
  if ( this->records.size() < 2 )
  {
    vtkLabelRecord* derivRecord = vtkLabelRecord::New();
	derivRecord->Initialize( this->GetRecordAt(0)->Size(), 0.0 );
    derivRecord->SetTime( GetRecordRT()->GetTime() );
    derivRecord->SetLabel( GetRecordRT()->GetLabel() );
    
	return derivRecord;
  }

  if ( order == 0 )
  {
    return GetRecordRT();
  }

  if ( order == 1 )
  {
    double DT = GetRecordAt( this->GetNumRecords() - 1 )->GetTime() - GetRecordAt( this->GetNumRecords() - 2 )->GetTime();
	vtkLabelRecord* derivRecord = vtkLabelRecord::New();

    for( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
    {
      derivRecord->Add( ( GetRecordAt( this->GetNumRecords() - 1 )->Get(d) - GetRecordAt( this->GetNumRecords() - 2 )->Get(d) ) / DT );
    }
	
    derivRecord->SetTime( GetRecordAt( this->GetNumRecords() - 1 )->GetTime() );
    derivRecord->SetLabel( GetRecordAt( this->GetNumRecords() - 1 )->GetLabel() );
    
	return derivRecord;
  }

  vtkRecordBuffer* derivRecordBuffer = this->Derivative( order );
  return derivRecordBuffer->GetCurrentRecord();

}



vtkLabelRecord* vtkRecordBufferRT
::GaussianFilterRT( double width )
{
  // Create a new record valuestor
  vtkLabelRecord* gaussRecord = vtkLabelRecord::New();
  gaussRecord->Initialize( this->GetRecordAt(0)->Size(), 0.0 );

  // Iterate over all dimensions
  for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
  {
    double weightSum = 0;
    double normSum = 0;
	double gaussWeight;
	double normDist;

    // Iterate over all records nearby
	int j = this->GetNumRecords() - 1;
	while ( j >= 0 ) // Iterate backward
    {
	  // If too far from "peak" of distribution, the stop - we're just wasting time
	  normDist = ( GetRecordAt(j)->GetTime() - GetRecordRT()->GetTime() ) / width;
	  if ( abs( normDist ) > STDEV_CUTOFF )
	  {
	    break;
	  }

      // Calculate the values of the Gaussian distribution at this time
	  gaussWeight = exp( - normDist * normDist / 2 );
	  // Add the product with the values to function sum
      weightSum = weightSum + GetRecordAt(j)->Get(d) * gaussWeight;
	  // Add the values to normSum
	  normSum = normSum + gaussWeight;

	  j--;
    }

    // Add to the new values
    gaussRecord->Set( d, weightSum / normSum );

  }

  // Add the new record vector to the record log
  gaussRecord->SetTime( GetRecordRT()->GetTime() );
  gaussRecord->SetLabel( GetRecordRT()->GetLabel() );

  return gaussRecord;
}



vtkLabelRecord* vtkRecordBufferRT
::OrthogonalTransformationRT( int window, int order )
{
  // Pad the recordlog with values at the beginning (only if necessary)
  // Do not initialize, will be set by the condition below
  vtkRecordBuffer* padRecordBuffer;
  vtkRecordBuffer* padCatRecordBuffer;
  if ( this->GetNumRecords() <= window )
  {
    padRecordBuffer = this->PadStart( window );
	padCatRecordBuffer = padRecordBuffer->Concatenate( this );
  }
  else
  {
    padRecordBuffer = NULL;
    padCatRecordBuffer = this;
  }

  // Calculate the record log to include
  vtkRecordBuffer* trimRecordBuffer = padCatRecordBuffer->Trim( padCatRecordBuffer->GetNumRecords() - 1 - window, padCatRecordBuffer->GetNumRecords() - 1 );
	
  // Create a new matrix to which the Legendre coefficients will be assigned
  std::vector<vtkLabelVector*> legCoeffMatrix = trimRecordBuffer->LegendreTransformation( order );
  
  vtkLabelRecord* legRecord = vtkLabelRecord::New();
  legRecord->Initialize( this->GetRecordAt(0)->Size() * ( order + 1 ), 0.0 );

  // Calculate the Legendre coefficients: 2D -> 1D
  int count = 0;
  for ( int o = 0; o <= order; o++ )
  {
    for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
    {
      legRecord->Set( count, legCoeffMatrix[o]->Get(d) );
	  count++;
    }
  }

  // New value record to add to the record log
  legRecord->SetTime( GetRecordRT()->GetTime() );
  legRecord->SetLabel( GetRecordRT()->GetLabel() );


  if ( this->GetNumRecords() <= window )
  {
    padRecordBuffer->Delete();
	padCatRecordBuffer->Delete();
  }
  trimRecordBuffer->Delete();
  vtkDeleteVector( legCoeffMatrix );

  return legRecord;
}


vtkLabelRecord* vtkRecordBufferRT
::TransformPCART( std::vector<vtkLabelVector*> prinComps, vtkLabelVector* mean )
{
  // Create a vtkLabelRecord* for the transformed record log
  vtkLabelRecord* transRecord = vtkLabelRecord::New();
  transRecord->Initialize( prinComps.size(), 0.0 );

  // Initialize the components of the transformed time record to be zero
  for ( int o = 0; o < prinComps.size(); o++ )
  {	  
    // Iterate over all dimensions, and perform the transformation (ie vector multiplcation)
    for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	{
      transRecord->Crement( o, ( GetRecordRT()->Get(d) - mean->Get(d) ) * prinComps.at(o)->Get(d) );
	}
  }

  // Add the time record to the new transformed record log
  transRecord->SetTime( GetRecordRT()->GetTime() );
  transRecord->SetLabel( GetRecordRT()->GetLabel() );

  return transRecord;
}



vtkLabelRecord* vtkRecordBufferRT
::fwdkmeansTransformRT( std::vector<vtkLabelVector*> centroids )
{
  // Calculate closest cluster centroid to last
  // Find the record farthest from any centroid
  // Tricky way to cast vector of vtkLabelVector* to vector of ValeuRecord
  vtkLabelVector* centDist = this->DistancesRT( centroids );

  double currMinDist = centDist->Get(0);
  int currMinCentroid = 0;
  // Minimum for each point
  for ( int c = 0; c < centroids.size(); c++ )
  {
    if ( centDist->Get(c) < currMinDist )
	{
      currMinDist = centDist->Get(c);
	  currMinCentroid = c;
	}
  }

  centDist->Delete();

  vtkLabelRecord* clustRecord = vtkLabelRecord::New();
  clustRecord->Add( currMinCentroid );
  clustRecord->SetTime( GetRecordRT()->GetTime() );
  clustRecord->SetLabel( GetRecordRT()->GetLabel() );
  
  return clustRecord;
}


vtkMarkovRecord* vtkRecordBufferRT
::ToMarkovRecordRT()
{
  vtkMarkovRecord* markovRecord = vtkMarkovRecord::New();

  // We will assume that: label -> state, values[0] -> symbol
  markovRecord->SetState( this->GetRecordRT()->GetLabel() );
  markovRecord->SetSymbol( this->GetRecordRT()->Get(0) );

  return markovRecord;
}