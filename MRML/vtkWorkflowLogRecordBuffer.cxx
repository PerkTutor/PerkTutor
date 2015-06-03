

#include "vtkWorkflowLogRecordBuffer.h"

vtkStandardNewMacro( vtkWorkflowLogRecordBuffer );



// Constructors and Destructors --------------------------------------------------------------------

vtkWorkflowLogRecordBuffer
::vtkWorkflowLogRecordBuffer()
{
}


vtkWorkflowLogRecordBuffer
::~vtkWorkflowLogRecordBuffer()
{
}


// Conversion to/from transform buffers

vtkMRMLTransformBufferNode* vtkWorkflowLogRecordBuffer
::ToTransformBufferNode()
{
	vtkMRMLTransformBufferNode* transformBufferNode = vtkMRMLTransformBufferNode::New();

  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkLabelRecord* labelRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    if ( labelRecord == NULL )
    {
      continue;
    }
    
    vtkSmartPointer< vtkTransformRecord > transformRecord = vtkSmartPointer< vtkTransformRecord >::New();
    labelRecord->ToTransformRecord( transformRecord, vtkLabelRecord::QUATERNION_RECORD );
    if ( transformRecord == NULL )
    {
      continue;
    }
    
    transformBufferNode->AddTransform( transformRecord );
  }

  // We must convert the labels into messages
  std::string prevLabel = "";
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkLabelRecord* labelRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    if ( labelRecord == NULL )
    {
      continue;
    }
    
    if ( labelRecord->GetLabel().compare( prevLabel ) != 0 )
	  {
	    vtkSmartPointer< vtkMessageRecord > messageRecord = vtkSmartPointer< vtkMessageRecord >::New();
	    messageRecord->SetTime( labelRecord->GetTime() );
	    messageRecord->SetMessageString( labelRecord->GetLabel() );
	    transformBufferNode->AddMessage( messageRecord );
	    prevLabel = labelRecord->GetLabel();
	  }
  }

  return transformBufferNode;  
}


// Only use transforms with the correct transform name
// Only keep the transforms who have associated messages that are relevant for this tool
void vtkWorkflowLogRecordBuffer
::FromTransformBufferNode( vtkMRMLTransformBufferNode* newTransformBufferNode, std::string transformName, std::vector< std::string > relevantMessages )
{
  this->Clear();
  
  // Populate the list of finishing messages
  std::vector< std::string > finishingMessages;
  finishingMessages.push_back( "Done" );
  finishingMessages.push_back( "End" );
  finishingMessages.push_back( "Finished" );

  // Add the transforms
  for ( int i = 0; i < newTransformBufferNode->GetNumTransforms( transformName ); i++ )
  {
    vtkTransformRecord* transformRecord = vtkTransformRecord::SafeDownCast( newTransformBufferNode->GetTransformAtIndex( i, transformName ) );
    if ( transformRecord == NULL )
    {
      return;
    }    
  
    vtkSmartPointer< vtkLabelRecord > labelRecord = vtkSmartPointer< vtkLabelRecord >::New();
    labelRecord->FromTransformRecord( transformRecord, QUATERNION_RECORD );
    
    this->AddRecord( labelRecord );
  }

  // This is a NumTransforms by NumMessages order algorithm anyway...
  // This will work because the messages are sorted by increased time
  for ( int i = 0; i < newTransformBufferNode->GetNumMessages(); i++ )
  {
    // Only accept if message is relevant
    bool relevant = false;
    for ( int j = 0; j < relevantMessages.size(); j++ )
	  {
      if ( newTransformBufferNode->GetMessageAtIndex( i )->GetMessageString().compare( relevantMessages.at( j ) ) == 0 )
	    {
        relevant = true;
	    }
	  }
    for ( int j = 0; j < finishingMessages.size(); j++ )
    {
      if ( newTransformBufferNode->GetMessageAtIndex( i )->GetMessageString().compare( finishingMessages.at( j ) ) == 0 )
	    {
        relevant = true;
	    }
    }

	  // Skip if the message is not relevant 
    if ( ! relevant )
    {
      continue;
    }

	  for ( int j = 0; j < this->GetNumRecords(); j++ )
	  {
      vtkLabelRecord* labelRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
      if ( labelRecord == NULL )
      {
        continue;
      }
    
      if ( labelRecord->GetTime() > newTransformBufferNode->GetMessageAtIndex( i )->GetTime() )
	    {
        labelRecord->SetLabel( newTransformBufferNode->GetMessageAtIndex( i )->GetMessageString() );
	    }
	  }

  }

}



// Methods specific for workflow segmentation -------------------------------------------------------------

// Note: This is inclusive (end points will appear in the result)
vtkWorkflowLogRecordBuffer* vtkWorkflowLogRecordBuffer
::GetRange( int start, int end )
{
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > rangeRecordBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
  
  // Iterate over all tracking records in the current procedure, and add to new
  for ( int i = start; i <= end; i++ )
  {
    vtkSmartPointer< vtkLabelRecord > currRecord = vtkSmartPointer< vtkLabelRecord >::New();
    currRecord->Copy( vtkLabelRecord::SafeDownCast( this->GetRecord( i ) ) );
    rangeRecordBuffer->AddRecord( currRecord );
  }

  return rangeRecordBuffer;
}



void vtkWorkflowLogRecordBuffer
::Concatenate( vtkWorkflowLogRecordBuffer* otherRecordBuffer )
{

  // Ensure that the record size are the same
  if ( this->GetNumRecords() < 1 )
  {
    this->Copy( otherRecordBuffer );
    return;
  }
  if ( otherRecordBuffer->GetNumRecords() < 1 )
  {
    return;
  }
  if ( this->GetCurrentRecord()->GetVector()->Size() != otherRecordBuffer->GetCurrentRecord()->GetVector()->Size() )
  {
    return;
  }

  // Add all records from the other to this
  for( int i = 0; i < otherRecordBuffer->GetNumRecords(); i++ )
  {
    this->AddRecord( otherRecordBuffer->GetRecord( i )->DeepCopy() );
  }

}



void vtkWorkflowLogRecordBuffer
::ConcatenateValues( vtkWorkflowLogRecordBuffer* otherRecordBuffer )
{
  // Only works if the number of records are the same for both record logs
  if ( this->GetNumRecords() != otherRecordBuffer->GetNumRecords() )
  {
    return;
  }

  // Iterate over all records in both logs and stick them together
  vktSmartPointer< vtkWorkflowLogRecordBuffer > catRecordBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();

  for ( int i = start; i <= end; i++ )
  {
    vtkLabelRecord* thisRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    if ( thisRecord == NULL )
    {
      continue;
    }
    
    vtkLabelRecord* otherRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    if ( otherRecord == NULL )
    {
      continue;
    }

    thisRecord->GetVector()->Concatenate( otherRecord->GetVector() );
  }
  
}


vtkWorkflowLogRecordBuffer* vtkWorkflowLogRecordBuffer
::ConcatenateValues( vtkLabelVector* vector )
{
  // Iterate over all records in this log and stick the vector onto each
  vktSmartPointer< vtkWorkflowLogRecordBuffer > catRecordBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();

  for ( int i = start; i <= end; i++ )
  {
    vtkLabelRecord* thisRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    if ( thisRecord == NULL )
    {
      continue;
    }
    
    thisRecord->GetVector()->Concatenate( vector );
  }
  
}



// Note: This adds the padding to the start of the current buffer
void vtkWorkflowLogRecordBuffer
::PadStart( int window )
{
  // Find the average time stamp
  // Divide by numRecords - 1 because there are one fewer differences than there are stamps
  double DT = 1.0;
  if ( this->GetNumRecords() > 1 )
  {
    DT = this->GetTotalTime() / ( this->GetNumRecords() - 1 );
  }

  // Calculate the values and time stamp
  vtkLabelRecord* initialRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( 0 ) );
  if ( initialRecord == NULL )
  {
    return;
  }
  
  // It will be sorted automatically
  // But complexity will be better if we are always adding to the front
  for ( int i = 1; i <= window; i++ )
  {
    vtkSmartPointer< vtkLabelRecord > currRecord = vtkSmartPointer< vtkLabalRecord >::New();
    currRecord->Copy( initialRecord );
    currRecord->SetTime( initialRecord->GetTime() - i * DT );
	  currRecord->SetLabel( initialRecord->GetLabel() );
	  this->AddRecord( currRecord );
  }

}


vtkLabelVector* vtkWorkflowLogRecordBuffer
::Mean()
{
  // The record log will only hold one record at the end
  int size = this->GetRecord( 0 )->GetVector()->Size();
  vtkSmartPointer< vtkLabelVector > meanVector = vtkSmartPointer< vtkLabelVector >::New();
  meanVector->Initialize( size, 0.0 );

  // For each time
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
	  // Iterate over all dimensions
	  for ( int d = 0; d < size; d++ )
	  {
	    meanVector->IncrementElement( d, this->GetRecordAt( i )->GetVector()->GetElement( d ) );
	  }
  }

  // Divide by the number of records
  for ( int d = 0; d < size; d++ )
  {
    meanVector->SetElement( d, meanVector->GetElement( d ) / this->GetNumRecords() );
  }

  return meanVector;
}



std::vector< vtkLabelVector* > vtkWorkflowLogRecordBuffer
::Distances( vtkWorkflowLogRecordBuffer* otherRecordBuffer )
{
  // Put the other record log into a vector
  std::vector< vtkSmartPointer< vtkLabelVector > > vectors = std::vector< vtkSmartPointer< vtkLabelVector > >( otherRecordBuffer->GetNumRecords() );
  for ( int = 0; i < otherRecordBuffer->GetNumRecords(); i++ )
  {
    vtkLabelRecord* labelRecord = vtkLabelRecord::SafeDownCast( otherRecordBuffer->GetRecord( i ) );
    if ( labelRecord == NULL )
    {
      continue;
    }
    
    vectors.at(i) = labelRecord->GetVector();
  }
  
  return this->Distances( vectors );
}



std::vector< vtkLabelVector* > vtkWorkflowLogRecordBuffer
::Distances( std::vector< vtkLabelVector* > vectors )
{
  // Create a vector of vectors
  std::vector< vtkSmartPointer< vtkLabelVector > > dists;

  // Otherwise, calculate the vectors based on distances from this's records
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    // Initialize values, so we don't change its size so many times
    vtkSmartPointer< vtkLabelVector > distVector = vtkSmartPointer< vtkLabelVector >::New();
	  distVector->Initialize( vectors.size(), 0.0 );

    for ( int j = 0; j < vectors.size(); j++ )
	  {      
      // First, ensure that the records are the same size
      if ( this->GetRecord( 0 )->GetVector()->Size() != vectors.at(j)->Size() )
      {
        return dists;
      }
      
      // Initialize the sum to zero
	    double currSum = 0.0;
	    for ( int d = 0; d < this->GetRecordAt( 0 )->GetVector()->Size(); d++ )
	    {
	      double currDiff = this->GetRecord( i )->GetVector()->GetElement( d ) - vectors.at(j)->GetElement( d );
        currSum += currDiff * currDiff;
	    }
	    
      // Add to the current order record
	    distVector->SetElement( j, sqrt( currSum ) );
	  }

	  // Add the current order record to the vector
	  distVector->SetLabel( this->GetRecordAt(i)->GetLabel() );
	  dists.push_back( distVector );
  }

  return dists;
}


// Calculate the record in the buffer that is closest to a particular point
vtkLabelRecord* vtkWorkflowLogRecordBuffer
::ClosestRecord( vtkLabelVector* vector )
{
  if ( this->GetRecord( 0 )->GetVector()->Size() != vector->Size() )
  {
    return vtkLabelRecord::New();
  }

  // Calculate the distance to this point
  std::vector< vtkLabelVector* > vectors( 1, vector );
  std::vector< vtkLabelVector* > dists = this->Distances( vectors );

  // Now find the closest point
  double minDist = std::numeric_limits< double >::max();
  vtkLabelRecord* minRecord = NULL;  
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    if ( dist.at(i)->GetElement( 0 ) < minDist )
	  {
      minDist = dist.at(i)->GetElement( 0 );
	    minRecord = this->GetRecord( i );
	  }
  }

  return minRecord;
}




void vtkWorkflowLogRecordBuffer
::Differentiate( int order )
{
  // If a derivative of order zero is required, then return a copy of this
  if ( this->GetNumRecords() < 2 || order == 0 )
  {
    return;
  }

  // Otherwise, calculate the derivative
  double DT;
  
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > derivRecordBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();

  // First (forward difference formula)
  vtkSmartPointer< vtkLabelRecord > derivFirstRecord = vtkSmartPointer< vtkLabelRecord >::New();
  vtkLabelRecord* firstRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( 0 ) );
  derivFirstRecord->Copy( firstRecord );
  
  DT = this->GetRecord( 1 )->GetTime() - this->GetRecord( 0 )->GetTime();
  for( int d = 0; d < this->GetRecord( 0 )->GetVector()->Size(); d++ )
  {
    derivFirstRecord->GetVector()->SetElement( d, ( this->GetRecord( 1 )->GetVector()->GetElement( d ) - this->GetRecord( 0 )->GetVector()->GetElement( d ) ) / DT );
  }

  derivRecordBuffer->AddRecord( derivFirstRecord );


  // Middle (centred difference formula)
  for ( int i = 1; i < this->GetNumRecords() - 1; i++ )
  {
  
    vtkSmartPointer< vtkLabelRecord > derivMiddleRecord = vtkSmartPointer< vtkLabelRecord >::New();
    vtkLabelRecord* middleRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    derivMiddleRecord->Copy( middleRecord );
  
    DT = this->GetRecord( i + 1 )->GetTime() - this->GetRecord( i - 1 )->GetTime();
    for( int d = 0; d < this->GetRecord( i )->GetVector()->Size(); d++ )
    {
      derivMiddleRecord->GetVector()->SetElement( d, ( this->GetRecord( i + 1 )->GetVector()->GetElement( d ) - this->GetRecord( i - 1 )->GetVector()->GetElement( d ) ) / DT );
    }

    derivRecordBuffer->AddRecord( derivMiddleRecord );
  }


  // Last (backward difference formula)
  vtkSmartPointer< vtkLabelRecord > derivLastRecord = vtkSmartPointer< vtkLabelRecord >::New();
  vtkLabelRecord* lastRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( this->GetNumRecord() - 1 ) );
  derivLastRecord->Copy( lastRecord );
  
  DT = this->GetRecord( this->GetNumRecord() - 1 )->GetTime() - this->GetRecord( this->GetNumRecord() - 2 )->GetTime();
  for( int d = 0; d < this->GetRecord( this->GetNumRecord() - 1 )->GetVector()->Size(); d++ )
  {
    derivLastRecord->GetVector()->SetElement( d, ( this->GetRecord( this->GetNumRecord() - 1 )->GetVector()->GetElement( d ) - this->GetRecord( this->GetNumRecord() - 2 )->GetVector()->GetElement( d ) ) / DT );
  }

  derivRecordBuffer->AddRecord( derivLastRecord );
  
  // Set this to be the differentiated buffer
  this->Copy( derivRecordBuffer );
  this->Differentiate( order - 1 );
}



vtkLabelVector* vtkWorkflowLogRecordBuffer
::Integrate()
{
  // The record log will only hold one record at the end
  int size = this->GetRecord( 0 )->GetVector()->Size();
  vtkSmartPointer< vtkLabelVector > intVector = vtkSmartPointer< vtkLabelVector >::New();
  intVector->Initialize( size, 0.0 );

  // For each time
  for ( int i = 1; i < this->GetNumRecords(); i++ )
  {
	  // Find the time difference
    double DT = this->GetRecord( i )->GetTime() - this->GetRecord( i - 1 )->GetTime();

	  // Iterate over all dimensions
    // This is the trapezoidal rule
	  for ( int d = 0; d < size; d++ )
	  {
	    intVector->Increment( d, DT * ( this->GetRecord( i )->GetVector()->GetElement( d ) + this->GetRecord( i - 1 )->GetVector()->GetElement( d ) ) / 2 );
	  }
  }

  return intVector;
}


std::vector< vtkLabelVector* > vtkWorkflowLogRecordBuffer
::LegendreTransformation( int order )
{
  // The time-shifted record buffer
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > shiftBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();

  // Calculate the time adjustment (need range -1 to 1)
  double startTime = this->GetRecordAt( 0 )->GetTime();
  double endTime = this->GetCurrentRecord()->GetTime();
  double rangeTime = endTime - startTime;
  
  if ( rangeTime <= 0 )
  {
    return
  }
  
  // Have to copy records to new buffer because sortedness is maintained on each buffer
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkLabelRecord* currRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    if ( currRecord == NULL )
    {
      return;
    }
  
    vtkSmartPointer< vtkLabelRecord > currShiftRecord = vtkSmartPointer< vtkLabelRecord >::New();
    currShiftRecord->Copy( currRecord );
    currShiftRecord->SetTime( 2.0 * ( currRecord->GetTime() - startTime ) / rangeTime - 1 ); // ( tmin, tmax ) --> ( -1, 1 )
    shiftRecordBuffer->AddRecord( currShiftRecord );
  }

  // Create a copy of the record log for each degree of Legendre polynomial
  std::vector< vtkSmartPointer< vtkLabelVector > > legendreCoefficientMatrix;

  for ( int o = 0; o <= order; o++ )
  {
	  vtkSmartPointer< vtkWorkflowLogRecordBuffer > unintegratedRecordBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();

    // Multiply the values by the Legendre polynomials
    for ( int i = 0; i < shiftRecordBuffer->GetNumRecords(); i++ )
    {
      double legendrePolynomial = LegendrePolynomial( shiftRecordBuffer->GetRecordAt(i)->GetTime(), o );

	    vtkSmartPointer< vtkLabelRecord > unintegratedRecord = vtkSmartPointer< vtkLabelRecord >::New();

      for ( int d = 0; d < this->GetRecord( i )->GetVector()->Size(); d++ )
	    {	    
        unintegratedRecord->GetVector()->AddElement( shiftRecordBuffer->GetRecord( i )->GetVector()->GetElement( d ) * legendrePolynomial );
	    }

	    unintegratedRecord->SetTime( shiftRecordBuffer->GetRecord( i )->GetTime() );
	    unintegratedRecord->SetLabel( shiftRecordBuffer->GetRecord( i )->GetLabel() );

	    unintRecordBuffer->AddRecord( unintegratedRecord );
    }

	  // Integrate to get the Legendre coefficients for the particular order
	  vtkSmartPointer< vtkLabelVector > legendreVector = unintegratedRecordBuffer->Integrate();
	  legendreVector->SetLabel( o );
	  legendreCoefficientMatrix.push_back( legendreVector );
  }

  return legendreCoefficientMatrix;
}



double vtkWorkflowLogRecordBuffer
::LegendrePolynomial( double time, int order )
{
  if ( order == 0 )
  {
    return 1;
  }
  if ( order == 1 )
  {
    return time;
  }
  if ( order == 2 )
  {
    return 3.0 / 2.0 * pow( time, 2.0 ) - 1.0 / 2.0;
  }
  if ( order == 3 )
  {
    return 5.0 / 2.0 * pow( time, 3.0 ) - 3.0 / 2.0 * time;
  }
  if ( order == 4 )
  {
    return 35.0 / 8.0 * pow( time, 4.0 ) - 15.0 / 4.0 * pow( time, 2.0 ) + 3.0 / 8.0;
  }
  if ( order == 5 )
  {
    return 63.0 / 8.0 * pow( time, 5.0 ) - 35.0 / 4.0 * pow( time, 3.0 ) + 15.0 / 8.0 * time;
  }
  if ( order == 6 )
  {
    return 231.0 / 16.0 * pow( time, 6.0 ) - 315.0 / 16.0 * pow( time, 4.0 ) + 105.0 / 16.0 * pow( time, 2.0 ) - 5.0 / 16.0;
  }

  return 0.0;
}



void vtkWorkflowLogRecordBuffer
::GaussianFilter( double width )
{
  // Assume a Gaussian filter
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > gaussRecordBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();

  // For each record
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    // Create a new record    
    vtkSmartPointer< vtkLabelRecord > gaussRecord = vtkSmartPointer< vtkLabelRecord >::New();
	  gaussRecord->GetVector()->Initialize( this->GetRecord( i )->GetVector()->Size(), 0.0 );

    // Iterate over all dimensions
	  for ( int d = 0; d < this->GetRecord( i )->GetVector()->Size(); d++ )
	  {
      double weightSum = 0.0;
      double normSum = 0.0;

      // Iterate over all records nearby to the left
	    int j = i;
	    while ( j >= 0 ) // Iterate backward
      {
	      // If too far from "peak" of distribution, the stop - we're just wasting time
	      double normalizedDistance = ( this->GetRecord( j )->GetTime() - this->GetRecord( i )->GetTime() ) / width;
		    if ( abs( normalizedDistance ) > STDEV_CUTOFF )
		    {
		      break;
		    }
        
        // Calculate the values of the Gaussian distribution at this time
	      double gaussianWeight = exp( - normalizedDistance * normalizedDistance / 2 );
		    // Add the product with the values to function sum
        weightSum = weightSum + this->GetRecord( j )->GetVector()->GetElement( d ) * gaussianWeight;
		    // Add the values to normSum
		    normSum = normSum + gaussianWeight;

		    j--;
      }

	    // Iterate over all records nearby to the right
	    j = i + 1;
	    while ( j < this->GetNumRecords() ) // Iterate forward
      {
	      // If too far from "peak" of distribution, the stop - we're just wasting time
	      double normalizedDistance = ( this->GetRecord( j )->GetTime() - this->GetRecord( i )->GetTime() ) / width;
		    if ( abs( normalizedDistance ) > STDEV_CUTOFF )
		    {
		      break;
		    }
        
        // Calculate the values of the Gaussian distribution at this time
	      double gaussianWeight = exp( - normalizedDistance * normalizedDistance / 2 );
		    // Add the product with the values to function sum
        weightSum = weightSum + this->GetRecord( j )->GetVector()->GetElement( d ) * gaussianWeight;
		    // Add the values to normSum
		    normSum = normSum + gaussianWeight;

		    j++;
      }

	    // Add to the new values
	    gaussRecord->GetVector()->SetElement( d, weightSum / normSum );

	  }

	  // Add the new record vector to the record log
	  gaussRecord->SetTime( this->GetRecord( i )->GetTime() );
	  gaussRecord->SetLabel( this->GetRecord( i )->GetLabel() );
    gaussRecordBuffer->AddRecord( gaussRecord );

  }

  this->Copy( gaussRecordBuffer );
}



void vtkWorkflowLogRecordBuffer
::OrthogonalTransformation( int window, int order )
{
  // Pad the RecordBuffer with values at the beginning
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > paddedBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
  paddedBuffer->Copy( this );
  paddedBuffer->PadStart( window );

  // Create a new record log with the orthogonally transformed data
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > orthogonalRecordBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();

  // Iterate over all data, and calculate Legendre expansion coefficients
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkSmartPointer< vtkLabelRecord > currLegendreRecord = vtkSmartPointer< vtkLabelRecord >::New();
    currLegendreRecord->Initialize( this->GetRecord( i )->Size() * ( order + 1 ), 0.0 );

    // Calculate the record log to include
    vtkSmartPointer< vtkWorkflowLogRecordBuffer > rangeBuffer = paddedBuffer->GetRange( i, i + window );
	  std::vector< vtkSmartPointer< vtkLabelVector > > legendreCoefficientMatrix = rangeBuffer->LegendreTransformation( order );

	  // Calculate the Legendre coefficients: 2D -> 1D
	  int count = 0;
	  for ( int o = 0; o <= order; o++ )
    {
      for ( int d = 0; d < this->GetRecord( i )->GetVector()->Size(); d++ )
	    {
        currLegendreRecord->SetElement( count, legendreCoefficientMatrix.at( o )->GetElement( d ) );
		    count++;
	    }
    }

	  // New value record to add to the record log
    currLegendreRecord->SetTime( this->GetRecord( i )->GetTime() );
	  currLegendreRecord->SetLabel( this->GetRecord( i )->GetLabel() );
	  orthogonalRecordBuffer->AddRecord( currLegendreRecord );
  }

  this->Copy( orthogonalRecordBuffer );
}


vnl_matrix<double>* vtkWorkflowLogRecordBuffer
::CovarianceMatrix()
{
  // Copy the current record log
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > zeroMeanBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();

  // Construct a recordSize by recordSize vnl matrix
  vnl_matrix<double>* covariance = new vnl_matrix<double>( this->GetRecord( 0 )->GetVector()->Size(), this->GetRecord( 0 )->GetVector()->Size() );
  covariance->fill( 0.0 );

  // Determine the mean, subtract the mean from each record
  vtkSmartPointer< vtkLabelVector > meanVector = this->Mean();

  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkSmartPointer< vtkLabelRecord > zeroMeanRecord = vtkLabelRecord::New();
    vtkLabelRecord* currRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    if ( currRecord == NULL )
    {
      continue;
    }
    zeroMeanRecord->Copy( currRecord );

    for( int d = 0; d < zeroMeanRecord->GetVector()->Size(); d++ )
	  {
	    zeroMeanRecord->GetVector()->Increment( d, meanVector->GetElement( d ) );
	  }

	  zeroMeanBuffer->AddRecord( zeroMeanRecord );
  }

  // Pick two dimensions, and find their covariance
  for ( int d1 = 0; d1 < this->GetRecord( 0 )->GetVector()->Size(); d1++ )
  {
    for ( int d2 = 0; d2 < this->GetRecord( 0 )->GetVector()->Size(); d2++ )
	  {
	    // Iterate over all times
	    for ( int i = 0; i < this->GetNumRecords(); i++ )
	    {
	      covariance->put( d1, d2, covariance->get( d1, d2 ) + zeroMeanBuffer->GetRecord( i )->GetVector()->GetElement( d1 ) * zeroMeanBuffer->GetRecord( i )->GetVector()->GetElement( d2 ) );
	    }
	    // Divide by the number of records
	    covariance->put( d1, d2, covariance->get( d1, d2 ) / zeroMeanBuffer->GetNumRecords() );
	  }
  }

  return covariance;
}



std::vector< vtkLabelVector* > vtkWorkflowLogRecordBuffer
::CalculatePCA( int numComp )
{
  // Calculate the covariance matrix
  vnl_matrix<double>* covariance = this->CovarianceMatrix();

  //Calculate the eigenvectors of the covariance matrix
  vnl_matrix<double> eigenvectors( covariance->rows(), covariance->cols(), 0.0 );
  vnl_vector<double> eigenvalues( covariance->rows(), 0.0 );
  vnl_symmetric_eigensystem_compute( *covariance, eigenvectors, eigenvalues );
  // Note: eigenvectors are ordered in increasing eigenvalue ( 0 = smallest, end = biggest )

  // Grab only the most important eigenvectors
  std::vector< vtkSmartPointer< vtkLabelVector > > prinComps;

  // Prevent more prinicipal components than original dimensions
  if ( numComp > eigenvectors.cols() )
  {
    numComp = eigenvectors.cols();
  }

  for ( int i = eigenvectors.cols() - 1; i > eigenvectors.cols() - 1 - numComp; i-- )
  {
    vtkSmartPointer< vtkLabelVector > currPrinComp = vtkSmartPointer< vtkLabelVector >::New();
    currPrinComp->Initialize( eigenvectors.rows(), 0.0 );
    
    for ( int d = 0; d < eigenvectors.rows(); d++ )
	  {
	    currPrinComp->SetElement( d, eigenvectors.get( d, i ) );
	  }

    currPrinComp->SetLabel( eigenvectors.cols() - 1 - i );
	  prinComps.push_back( currPrinComp );
  }

  return prinComps;
}



void vtkWorkflowLogRecordBuffer
::TransformPCA( std::vector< vtkLabelVector* > prinComps, vtkLabelVector* meanVector )
{
  // Iterate over all time stamps
  for( int i = 0; i < this->GetNumRecords(); i++ )
  {
    // Create a vtkLabelRecord* for the transformed record log
    vtkLabelRecord* currRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    if ( currRecord == NULL )
    {
      continue;
    }
    vtkSmartPointer< vtkLabelRecord > pcaTransformRecord = vtkSmartPointer< vtkLabelRecord >::New();
    pcaTransformRecord->GetVector()->Initialize( prinComps.size(), 0.0 );
    
    // Initialize the components of the transformed time record to be zero
	  for ( int o = 0; o < prinComps.size(); o++ )
	  {
	    // Iterate over all dimensions, and perform the transformation (i.e. vector multiplication)
      for ( int d = 0; d < currRecord->GetVector()->Size(); d++ )
	    {
        pcaTransRecord->Increment( o, ( currRecord->GetVector()->GetElement( d ) - meanVector->GetElement( d ) ) * prinComps.at(o)->GetElement( d ) );
	    }
	  }

    // Copy the transformed values into the current record, which is held onto by the current buffer
    currRecord->SetAllValues( pcaTransformRecord->GetAllValues() );
  }

}



std::vector< vtkLabelVector* > vtkWorkflowLogRecordBuffer
::fwdkmeans( int numClusters )
{
  // Create a new vector of centroids
  std::vector< vtkSmartPointer< vtkLabelVector > > centroids;

  // A vector of cluster memberships
  std::vector< int > membership( this->GetNumRecords(), 0 );

  // Iterate until all of the clusters have been added
  for ( int k = 0; k < numClusters; k++ )
  {

	  // Use closest point to the mean of all points for the first centroid
    if ( k == 0 )
	  {
	    // An order record for the current cluster
      vtkSmartPointer< vtkLabelVector > currCentroid = this->Mean();
	    currCentroid->SetLabel( k );
	    centroids.push_back( currCentroid );
	    continue;
	  }

	  centroids.push_back( this->FindNextCentroid( centroids ) );

	  // Iterate until there are no more changes in membership and no clusters are empty
    bool change = true;
	  while ( change )
	  {
	    // Reassign the cluster memberships
	    std::vector<int> newMembership = this->ReassignMembership( centroids );

	    // Calculate change
	    change = this->MembershipChanged( membership, newMembership );
      if ( ! change )
	    {
        break;
	    }
	    membership = newMembership;

	    // Remove emptiness
	    std::vector< bool > emptyVector = this->FindEmptyClusters( centroids, membership );
      if ( this->HasEmptyClusters( emptyVector ) )
	    {
        centroids = this->MoveEmptyClusters( centroids, emptyVector );
		    // At the end of this, we are guaranteed no empty clusters
        continue;
	    }	  

	    // Recalculate centroids
	    centroids = this->RecalculateCentroids( membership, k + 1 );

	  }

  }

  return centroids;

}




vtkLabelVector* vtkWorkflowLogRecordBuffer
::FindNextCentroid( std::vector< vtkLabelVector* > centroids )
{
  // Find the record farthest from any centroid
  std::vector< vtkSmartPointer< vtkLabelVector > > centDist = this->Distances( centroids );
	
  int candidateRecord = 0;
  double candidateDistance = 0;

  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    double currMinDist = std::numeric_limits< double >::max();
    // Minimum for each point
    for ( int c = 0; c < centroids.size(); c++ )
	  {
      if ( centDist.at(i)->GetElement( c ) < currMinDist )
	    {
        currMinDist = centDist.at(i)->GetElement( c );
	    }
	  }
	
	  // Maximum of the minimums
    if ( currMinDist > candidateDistance )
    {
      candidateDistance = currMinDist;
      candidateRecord = i;
	  }
  }

  // Create new centroid for candidate
  vtkSmartPointer< vtkLabelVector > currCentroid = vtkSmartPointer< vtkLabelVector >::New();
  currCentroid->SetAllValues( this->GetRecord( candidateRecord )->GetAllValues() );
  currCentroid->SetLabel( centroids.size() );
  return currCentroid;
}



bool vtkWorkflowLogRecordBuffer
::MembershipChanged( std::vector< int > oldMembership, std::vector< int > newMembership )
{
  for ( int i = 0; i < oldMembership.size(); i++ )
  {
    if ( oldMembership.at( i ) != newMembership.at( i ) )
	  {
      return true;
	  }
  }

  return false;
}



std::vector<bool> vtkWorkflowLogRecordBuffer
::FindEmptyClusters( std::vector< vtkLabelVector* > centroids, std::vector< int > membership )
{
  std::vector< bool > emptyVector( centroids.size(), true );

  // Calculate the empty clusters
  for ( int i = 0; i < membership.size(); i++ )
  {
    emptyVector.at( membership.at(i) ) = false;
  }

  return emptyVector;
}



bool vtkWorkflowLogRecordBuffer
::HasEmptyClusters( std::vector< bool > emptyVector )
{
  for ( int c = 0; c < emptyVector.size(); c++ )
  {
    if ( emptyVector.at( i ) )
	  {
      return true;
	  }
  }
 
  return false;
}



std::vector< vtkLabelVector* > vtkWorkflowLogRecordBuffer
::MoveEmptyClusters( std::vector< vtkLabelVector* > centroids, std::vector< bool > emptyVector )
{
  // Remove any emptyness
  for ( int c = 0; c < centroids.size(); c++ )
  {
    if ( emptyVector.at(c) == false )
	  {
	    continue;
	  }

	  centroids.at(c) = this->FindNextCentroid( centroids );
  }

  return centroids;
}


std::vector< int > vtkWorkflowLogRecordBuffer
::ReassignMembership( std::vector< vtkLabelVector* > centroids )
{
  // Find the record farthest from any centroid
  std::vector< vtkSmartPointer< vtkLabelVector > > centDist = this->Distances( centroids );
  
  std::vector< int > membership;

  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    double currMinDist = std::numeric_limits< double >::max();
	  int currMinCentroid = 0;
    // Minimum for each point
    for ( int c = 0; c < centroids.size(); c++ )
	  {
      if ( centDist.at(i)->GetElement( c ) < currMinDist )
	    {
        currMinDist = centDist.at(i)->GetElement( c );
		    currMinCentroid = c;
	    }
	  }
    
    membership.push_back( currMinCentroid );
  }

  return membership;
}



std::vector< vtkLabelVector* > vtkWorkflowLogRecordBuffer
::RecalculateCentroids( std::vector< int > membership, int numClusters )
{

  // For each cluster, have an order record and a count
  std::vector< vtkSmartPointer< vtkLabelVector > > centroids;
  std::vector< int > memberCount( numClusters, 0 );

  // Initialize the list of centroids
  for ( int c = 0; c < numClusters; c++ )
  {
    vtkSmartPointer< vtkLabelVector > currCentroid = vtkSmartPointer< vtkLabelVector >::New();
	  currCentroid->Initialize( this->GetRecord( 0 )->GetVector()->Size(), 0.0 );
	  centroids.push_back( currCentroid );
  }

  // Iterate over all time
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    // For each dimension
    for ( int d = 0; d < this->GetRecord( i )->GetVector()->Size(); d++ )
	  {
	    centroids.at( membership.at(i) )->Increment( d, this->GetRecord( i )->GetVector()->GetElement( d ) );
	  }
    
    memberCount.at( membership.at(i) )++;
  }

  // Divide by the number of records in the cluster to get the mean
  for ( int c = 0; c < numClusters; c++ )
  {
    // For each dimension
    for ( int d = 0; d < centroid.at( c )->Size(); d++ )
	  {
	    centroids.at( c )->SetElement( d, centroids.at( c )->GetElement( d ) / memberCount.at( c ) );
	  }
	  centroids.at(c)->SetLabel( c );
  }

  return centroids;
}



void vtkWorkflowLogRecordBuffer
::fwdkmeansTransform( std::vector< vtkLabelVector* > centroids )
{
  // Use the reassign membership function to calculate closest centroids
  std::vector< int > membership = this->ReassignMembership( centroids );

  // Iterate over all tracking records in the current procedure, and add to new
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkLabelRecord* currRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    if ( currRecord == NULL )
    {
      continue;
    }
    
    std::vector< double > currMembership( 1, membership.at( i ) );
    currRecord->SetAllValues( currMembership );
  }

}


vtkWorkflowLogRecordBuffer* vtkWorkflowLogRecordBuffer
::GetLabelledRange( std::vector< std::string > labels )
{
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > rangeRecordBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();

  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkSmartPointer< vtkLabelRecord > currRecord = vtkSmartPointer< vtkLabelRecord >::New();
    if ( currRecord == NULL )
    {
      continue;
    }
    currRecord->Copy( vtkLabelRecord::SafeDownCast( this->GetRecord( i ) ) );
    
    // Check if the current record satisfies one of the labels
    for ( int j = 0; j < labels.size(); j++ )
	  {
      if ( labels.at(j).compare( currRecord->GetVector()->GetLabel() ) == 0 )
	    {        
        rangeRecordBuffer->AddRecord( currRecord );
	    }
	  }
    
  }

  return rangeRecordBuffer;
}


std::vector< vtkMarkovVector* > vtkWorkflowLogRecordBuffer
::ToMarkovVectors()
{
  std::vector< vtkSmartPointer< vtkMarkovRecord > > markovVectors;

  // We will assume that: label -> state, values[0] -> symbol
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkSmartPointer< vtkMarkovRecord > currMarkovVector = vtkSmartPointer< vtkMarkovRecord >::New();
    currMarkovVector->SetState( this->GetRecord( i )->GetLabel() );
	  currMarkovVector->SetSymbol( this->GetRecord(i)->GetVector()->GetElement( 0 ) );
	  markovVectors.push_back( currMarkovVector );
  }

  return markovVectors;
}