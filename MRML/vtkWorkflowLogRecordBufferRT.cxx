
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


vtkLabelVector* vtkWorkflowLogRecordBufferRT
::DistancesRT( std::vector< vtkSmartPointer< vtkLabelVector > > vectors )
{ 
  // Create a new order record
  vtkSmartPointer< vtkLabelVector > distVector = vtkSmartPointer< vtkLabelVector >::New();
  distVector->FillElements( vectors.size(), 0.0 );
  
  // Get the "current" record
  vtkLabelRecord* rtRecord = vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() );
  if ( rtRecord == NULL )
  {
    return distVector;  
  }

  double currSum;

  for ( int j = 0; j < vectors.size(); j++ )
  {      
    // First, ensure that the records are the same size
    if ( rtRecord->GetVector()->Size() != vectors.at( j )->Size() )
    {
      return distVector;
    }

    // Initialize the sum to zero
    currSum = 0.0;
    for ( int d = 0; d < rtRecord->GetVector()->Size(); d++ )
    {
      double currDiff = rtRecord->GetVector()->GetElement( d ) - vectors[j]->GetElement( d );
      currSum += currDiff * currDiff;
	  }
	  
    // Add to the current order record
	  distVector->SetElement( j, currSum );
  }

  // Add the current order record to the vector
  distVector->SetLabel( 0 );

  return distVector;

}


vtkLabelRecord* vtkWorkflowLogRecordBufferRT
::DifferentiateRT( int order )
{
  // To calculate a derivative of arbitrary order, we need arbitrarily many time stamps
  // Assume that order is constant
  if ( this->GetNumRecords() < order + 1 )
  {
    return vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() );
  }
  
  // Just need the last order + 1 timestamps
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > endBuffer = this->GetRange( this->GetNumRecords() - ( order + 1 ), this->GetNumRecords() - 1 );
  endBuffer->Differentiate( order );
  
  return vtkLabelRecord::SafeDownCast( endBuffer->GetCurrentRecord() );
}



vtkLabelRecord* vtkWorkflowLogRecordBufferRT
::GaussianFilterRT( double width )
{
  // Create a new record valuestor
  vtkSmartPointer< vtkLabelRecord > gaussRecord = vtkSmartPointer< vtkLabelRecord >::New();
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

  return gaussRecord;
}



vtkLabelRecord* vtkWorkflowLogRecordBufferRT
::OrthogonalTransformationRT( int window, int order )
{
  // Pad the recordlog with values at the beginning only if necessary
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > rangeBuffer;
  
  if ( this->GetNumRecords() <= window )
  {
    vtkSmartPointer< vtkWorkflowLogRecordBuffer > paddedBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
    paddedBuffer->Copy( this );
    paddedBuffer->PadStart( window );
    rangeBuffer = paddedBuffer->GetRange( paddedBuffer->GetNumRecords() - ( window + 1 ), paddedBuffer->GetNumRecords() - 1 );
  }
  else
  {
    rangeBuffer = this->GetRange( this->GetNumRecords() - ( window + 1 ), this->GetNumRecords() - 1 );
  }

  // Create a new matrix to which the Legendre coefficients will be assigned
  std::vector< vtkSmartPointer< vtkLabelVector > > legendreCoefficientMatrix = rangeBuffer->LegendreTransformation( order );
  
  vtkSmartPointer< vtkLabelRecord > currLegendreRecord = vtkSmartPointer< vtkLabelRecord >::New();
  currLegendreRecord->GetVector()->FillElements( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() )->GetVector()->Size() * ( order + 1 ), 0.0 );

  // Calculate the Legendre coefficients: 2D -> 1D
  int count = 0;
  for ( int o = 0; o <= order; o++ )
  {
    for ( int d = 0; d < legendreCoefficientMatrix.at( o )->Size(); d++ )
    {
      currLegendreRecord->GetVector()->SetElement( count, legendreCoefficientMatrix.at( o )->GetElement( d ) );
      count++;
    }
  }

  // New value record to add to the record log
  currLegendreRecord->SetTime( this->GetCurrentRecord()->GetTime() );
  currLegendreRecord->GetVector()->SetLabel( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() )->GetVector()->GetLabel() );

  return currLegendreRecord;
}


vtkLabelRecord* vtkWorkflowLogRecordBufferRT
::TransformPCART( std::vector< vtkSmartPointer< vtkLabelVector > > prinComps, vtkLabelVector* mean )
{

  // Create a vtkLabelRecord* for the transformed record log
  vtkLabelRecord* currRecord = vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() );
  if ( currRecord == NULL )
  {
    return NULL;
  }
  vtkSmartPointer< vtkLabelRecord > pcaTransformRecord = vtkSmartPointer< vtkLabelRecord >::New();
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

  return pcaTransformRecord;
}



vtkLabelRecord* vtkWorkflowLogRecordBufferRT
::fwdkmeansTransformRT( std::vector< vtkSmartPointer< vtkLabelVector > > centroids )
{
  // Calculate closest cluster centroid to last
  // Find the record farthest from any centroid
  vtkSmartPointer< vtkLabelVector > centDist = this->DistancesRT( centroids );

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

  vtkSmartPointer< vtkLabelRecord > clusterRecord = vtkSmartPointer< vtkLabelRecord >::New();
  clusterRecord->Copy( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() ) );
  std::vector< double > membership( 1, currMinCentroid );
  clusterRecord->GetVector()->SetAllValues( membership );

  return clusterRecord;
}


vtkMarkovVector* vtkWorkflowLogRecordBufferRT
::ToMarkovVectorRT()
{
  vtkSmartPointer< vtkMarkovVector > markovVector = vtkSmartPointer< vtkMarkovVector >::New();

  // We will assume that: label -> state, values[0] -> symbol
  markovVector->SetState( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() )->GetVector()->GetLabel() );
  markovVector->SetSymbol( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() )->GetVector()->GetElement( 0 ) );

  return markovVector;
}