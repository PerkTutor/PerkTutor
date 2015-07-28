
#include "vtkWorkflowLogRecordBufferRT.h"

vtkStandardNewMacro( vtkWorkflowLogRecordBufferRT );

vtkWorkflowLogRecordBufferRT
::vtkWorkflowLogRecordBufferRT()
{
}


vtkWorkflowLogRecordBufferRT
::~vtkWorkflowLogRecordBufferRT()
{
}


void vtkWorkflowLogRecordBufferRT
::DistancesRT( std::vector< vtkSmartPointer< vtkLabelVector > > vectors, vtkLabelVector* distanceVector )
{ 
  // Create a new order record
  distanceVector->FillElements( vectors.size(), 0.0 );
  
  // Get the "current" record
  vtkLabelRecord* rtRecord = vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() );
  if ( rtRecord == NULL )
  {
    return;  
  }

  double currSum;

  for ( int j = 0; j < vectors.size(); j++ )
  {      
    // First, ensure that the records are the same size
    if ( rtRecord->GetVector()->Size() != vectors.at( j )->Size() )
    {
      return;
    }

    // Initialize the sum to zero
    currSum = 0.0;
    for ( int d = 0; d < rtRecord->GetVector()->Size(); d++ )
    {
      double currDiff = rtRecord->GetVector()->GetElement( d ) - vectors[j]->GetElement( d );
      currSum += currDiff * currDiff;
	  }
	  
    // Add to the current order record
	  distanceVector->SetElement( j, currSum );
  }

  // Add the current order record to the vector
  distanceVector->SetLabel( 0 );

}


void vtkWorkflowLogRecordBufferRT
::DifferentiateRT( int order, vtkLabelRecord* diffRecord )
{
  // To calculate a derivative of arbitrary order, we need arbitrarily many time stamps
  // Assume that order is constant
  if ( this->GetNumRecords() < order + 1 )
  {
    diffRecord->Copy( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() ) );
    return;
  }
  
  // Just need the last order + 1 timestamps
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > endBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
  this->GetRange( this->GetNumRecords() - ( order + 1 ), this->GetNumRecords() - 1, endBuffer );
  endBuffer->Differentiate( order );
  
  diffRecord->Copy( vtkLabelRecord::SafeDownCast( endBuffer->GetCurrentRecord() ) );
}



void vtkWorkflowLogRecordBufferRT
::GaussianFilterRT( double width, vtkLabelRecord* gaussRecord )
{
  // Create a new record valuestor
  gaussRecord->GetVector()->FillElements( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() )->GetVector()->Size(), 0.0 );

  // Iterate over all dimensions
  for ( int d = 0; d < vtkLabelRecord::SafeDownCast( this->GetRecord( 0 ) )->GetVector()->Size(); d++ )
  {
    double weightSum = 0;
    double normSum = 0;

    // Iterate over all records nearby
	  int j = this->GetNumRecords() - 1;
	  while ( j >= 0 ) // Iterate backward
    {
	    // If too far from "peak" of distribution, the stop - we're just wasting time
	    double normalizedDistance = ( this->GetRecord( j )->GetTime() - this->GetCurrentRecord()->GetTime() ) / width;
	    if ( abs( normalizedDistance ) > STDEV_CUTOFF )
	    {
	      break;
	    }

      // Calculate the values of the Gaussian distribution at this time
	    double gaussianWeight = exp( - normalizedDistance * normalizedDistance / 2 );
	    // Add the product with the values to function sum
      weightSum = weightSum + vtkLabelRecord::SafeDownCast( this->GetRecord( j ) )->GetVector()->GetElement( d ) * gaussianWeight;
	    // Add the values to normSum
	    normSum = normSum + gaussianWeight;

	    j--;
    }

    // Add to the new values
    gaussRecord->GetVector()->SetElement( d, weightSum / normSum );
  }

  // Add the new record vector to the record log
  gaussRecord->SetTime( this->GetCurrentRecord()->GetTime() );
  gaussRecord->GetVector()->SetLabel( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() )->GetVector()->GetLabel() );
}



void vtkWorkflowLogRecordBufferRT
::OrthogonalTransformationRT( int window, int order, vtkLabelRecord* orthogonalRecord )
{
  // Pad the recordlog with values at the beginning only if necessary
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > rangeBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
  
  if ( this->GetNumRecords() <= window )
  {
    vtkSmartPointer< vtkWorkflowLogRecordBuffer > paddedBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
    paddedBuffer->Copy( this );
    paddedBuffer->PadStart( window );
    paddedBuffer->GetRange( paddedBuffer->GetNumRecords() - ( window + 1 ), paddedBuffer->GetNumRecords() - 1, rangeBuffer );
  }
  else
  {
    this->GetRange( this->GetNumRecords() - ( window + 1 ), this->GetNumRecords() - 1, rangeBuffer );
  }

  // Create a new matrix to which the Legendre coefficients will be assigned
  std::vector< vtkSmartPointer< vtkLabelVector > > legendreCoefficientMatrix;
  legendreCoefficientMatrix = rangeBuffer->LegendreTransformation( order );
  
  orthogonalRecord->GetVector()->FillElements( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() )->GetVector()->Size() * ( order + 1 ), 0.0 );

  // Calculate the Legendre coefficients: 2D -> 1D
  int count = 0;
  for ( int o = 0; o <= order; o++ )
  {
    for ( int d = 0; d < legendreCoefficientMatrix.at( o )->Size(); d++ )
    {
      orthogonalRecord->GetVector()->SetElement( count, legendreCoefficientMatrix.at( o )->GetElement( d ) );
      count++;
    }
  }

  // New value record to add to the record log
  orthogonalRecord->SetTime( this->GetCurrentRecord()->GetTime() );
  orthogonalRecord->GetVector()->SetLabel( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() )->GetVector()->GetLabel() );
}


void vtkWorkflowLogRecordBufferRT
::TransformPCART( std::vector< vtkSmartPointer< vtkLabelVector > > prinComps, vtkLabelVector* mean, vtkLabelRecord* pcaTransformRecord )
{
  // Create a vtkLabelRecord* for the transformed record log
  vtkLabelRecord* currRecord = vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() );
  if ( currRecord == NULL )
  {
    return;
  }
  pcaTransformRecord->GetVector()->FillElements( prinComps.size(), 0.0 );
    
  // Initialize the components of the transformed time record to be zero
	for ( int o = 0; o < prinComps.size(); o++ )
	{
	  // Iterate over all dimensions, and perform the transformation (i.e. vector multiplication)
    for ( int d = 0; d < currRecord->GetVector()->Size(); d++ )
	  {
      pcaTransformRecord->GetVector()->IncrementElement( o, ( currRecord->GetVector()->GetElement( d ) - mean->GetElement( d ) ) * prinComps.at(o)->GetElement( d ) );
	  }
	}

}



void vtkWorkflowLogRecordBufferRT
::fwdkmeansTransformRT( std::vector< vtkSmartPointer< vtkLabelVector > > centroids, vtkLabelRecord* fwdkmeansRecord )
{
  // Calculate closest cluster centroid to last
  // Find the record farthest from any centroid
  vtkSmartPointer< vtkLabelVector > centDist = vtkSmartPointer< vtkLabelVector >::New();
  this->DistancesRT( centroids, centDist );

  double currMinDist = std::numeric_limits< double >::max();
  int currMinCentroid = 0;
  // Minimum for each point
  for ( int c = 0; c < centroids.size(); c++ )
  {
    if ( centDist->GetElement( c ) < currMinDist )
	  {
      currMinDist = centDist->GetElement( c );
	    currMinCentroid = c;
	  }
  }

  fwdkmeansRecord->Copy( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() ) );
  std::vector< double > membership( 1, currMinCentroid );
  fwdkmeansRecord->GetVector()->SetAllValues( membership );
}


void vtkWorkflowLogRecordBufferRT
::ToMarkovVectorRT( vtkMarkovVector* markovVector )
{
  // We will assume that: label -> state, values[0] -> symbol
  markovVector->SetState( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() )->GetVector()->GetLabel() );
  markovVector->SetSymbol( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() )->GetVector()->GetElement( 0 ) );
}