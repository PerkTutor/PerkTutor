

#include "vtkWorkflowLogRecordBuffer.h"

vtkStandardNewMacro( vtkWorkflowLogRecordBuffer );


// Constants ---------------------------------------------------------------------------------------

const double vtkWorkflowLogRecordBuffer::STDEV_CUTOFF = 5.0;


// Constructors and Destructors --------------------------------------------------------------------

vtkWorkflowLogRecordBuffer
::vtkWorkflowLogRecordBuffer()
{
}


vtkWorkflowLogRecordBuffer
::~vtkWorkflowLogRecordBuffer()
{
}


// TODO: Can we just use the base class' copy method?
void vtkWorkflowLogRecordBuffer
::Copy( vtkWorkflowLogRecordBuffer* otherBuffer )
{
  if ( otherBuffer == NULL )
  {
    return;
  }

  // Copy all of the records
  this->Clear();
  for ( int i = 0; i < otherBuffer->GetNumRecords(); i++ )
  {
    vtkSmartPointer< vtkLabelRecord > newRecord = vtkSmartPointer< vtkLabelRecord >::New();
    newRecord->Copy( vtkLabelRecord::SafeDownCast( otherBuffer->GetRecord( i ) ) );
    this->AddRecord( newRecord );
  }

}


// Conversion to/from transform buffers

void vtkWorkflowLogRecordBuffer
::ToTransformBufferNode( vtkMRMLTransformBufferNode* transformBufferNode )
{
  transformBufferNode->Clear();

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
    
    if ( labelRecord->GetVector()->GetLabel().compare( prevLabel ) != 0 )
	  {
	    vtkSmartPointer< vtkMessageRecord > messageRecord = vtkSmartPointer< vtkMessageRecord >::New();
	    messageRecord->SetTime( labelRecord->GetTime() );
	    messageRecord->SetMessageString( labelRecord->GetVector()->GetLabel() );
	    transformBufferNode->AddMessage( messageRecord );
	    prevLabel = labelRecord->GetVector()->GetLabel();
	  }
  }
 
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
    labelRecord->FromTransformRecord( transformRecord, vtkLabelRecord::QUATERNION_RECORD );
    
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
      vtkLabelRecord* labelRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( j ) );
      if ( labelRecord == NULL )
      {
        continue;
      }
    
      if ( labelRecord->GetTime() > newTransformBufferNode->GetMessageAtIndex( i )->GetTime() )
	    {
        labelRecord->GetVector()->SetLabel( newTransformBufferNode->GetMessageAtIndex( i )->GetMessageString() );
	    }
	  }

  }

}


// In theory this should never be saved, but saving might be useful for debugging, so let's allow it for now
std::string vtkWorkflowLogRecordBuffer
::ToXMLString( vtkIndent indent )
{
  std::stringstream xmlstring;
  
  xmlstring << indent << "<WorkflowLogRecordBuffer>" << std::endl;

  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    xmlstring << this->GetRecord( i )->ToXMLString( indent.GetNextIndent() );
  }

  xmlstring << indent << "</WorkflowLogRecordBuffer>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowLogRecordBuffer
::FromXMLElement( vtkXMLDataElement* rootElement )
{
  if ( ! rootElement || strcmp( rootElement->GetName(), "WorkflowLogRecordBuffer" ) != 0 ) 
  {
    return;
  }

  int numElements = rootElement->GetNumberOfNestedElements();  // Number of saved records (including transforms and messages).
  
  for ( int i = 0; i < numElements; i++ )
  {
    vtkXMLDataElement* element = rootElement->GetNestedElement( i );

	  if ( element == NULL || strcmp( element->GetName(), "log" ) != 0 )
	  {
      continue;
	  }

    vtkSmartPointer< vtkLabelRecord > newRecord = vtkSmartPointer< vtkLabelRecord >::New();
	  newRecord->FromXMLElement( element );
	  this->AddRecord( newRecord );
  }

}



// Methods specific for workflow segmentation -------------------------------------------------------------

// Note: This is inclusive (end points will appear in the result)
void vtkWorkflowLogRecordBuffer
::GetRange( int start, int end, vtkWorkflowLogRecordBuffer* rangeRecordBuffer )
{
  rangeRecordBuffer->Clear();
  // Iterate over all tracking records in the current procedure, and add to new
  for ( int i = start; i <= end; i++ )
  {
    vtkSmartPointer< vtkLabelRecord > currRecord = vtkSmartPointer< vtkLabelRecord >::New();
    currRecord->Copy( vtkLabelRecord::SafeDownCast( this->GetRecord( i ) ) );
    rangeRecordBuffer->AddRecord( currRecord );
  }
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
  if ( vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() )->GetVector()->Size() != vtkLabelRecord::SafeDownCast( otherRecordBuffer->GetCurrentRecord() )->GetVector()->Size() )
  {
    return;
  }

  // Add all records from the other to this
  // If you want the records copied, copy the buffer first
  for( int i = 0; i < otherRecordBuffer->GetNumRecords(); i++ )
  {
    this->AddRecord( otherRecordBuffer->GetRecord( i ) );
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

  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkLabelRecord* thisRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    if ( thisRecord == NULL )
    {
      continue;
    }
    
    vtkLabelRecord* otherRecord = vtkLabelRecord::SafeDownCast( otherRecordBuffer->GetRecord( i ) );
    if ( otherRecord == NULL )
    {
      continue;
    }

    thisRecord->GetVector()->Concatenate( otherRecord->GetVector() );
  }
  
}


void vtkWorkflowLogRecordBuffer
::ConcatenateValues( vtkLabelVector* vector )
{
  // Iterate over all records in this log and stick the vector onto each
  for ( int i = 0; i < this->GetNumRecords(); i++ )
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
    vtkSmartPointer< vtkLabelRecord > currRecord = vtkSmartPointer< vtkLabelRecord >::New();
    currRecord->Copy( initialRecord );
    currRecord->SetTime( initialRecord->GetTime() - i * DT );
	  currRecord->GetVector()->SetLabel( initialRecord->GetVector()->GetLabel() );
	  this->AddRecord( currRecord );
  }

}


void vtkWorkflowLogRecordBuffer
::Mean( vtkLabelVector* meanVector )
{
  // The record log will only hold one record at the end
  int size = vtkLabelRecord::SafeDownCast( this->GetRecord( 0 ) )->GetVector()->Size();

  meanVector->FillElements( size, 0.0 );
  meanVector->SetLabel( "Mean" );

  // For each time
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
	  // Iterate over all dimensions
	  for ( int d = 0; d < size; d++ )
	  {
      meanVector->IncrementElement( d, vtkLabelRecord::SafeDownCast( this->GetRecord( i ) )->GetVector()->GetElement( d ) );
	  }
  }

  // Divide by the number of records
  for ( int d = 0; d < size; d++ )
  {
    meanVector->SetElement( d, meanVector->GetElement( d ) / this->GetNumRecords() );
  }

}



std::vector< vtkSmartPointer< vtkLabelVector > > vtkWorkflowLogRecordBuffer
::Distances( vtkWorkflowLogRecordBuffer* otherRecordBuffer )
{
  // Put the other record log into a vector
  std::vector< vtkSmartPointer< vtkLabelVector > > vectors = std::vector< vtkSmartPointer< vtkLabelVector > >( otherRecordBuffer->GetNumRecords() );
  for ( int i = 0; i < otherRecordBuffer->GetNumRecords(); i++ )
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



std::vector< vtkSmartPointer< vtkLabelVector > > vtkWorkflowLogRecordBuffer
::Distances( std::vector< vtkSmartPointer< vtkLabelVector > > vectors )
{
  // Create a vector of vectors
  std::vector< vtkSmartPointer< vtkLabelVector > > dists;

  // Otherwise, calculate the vectors based on distances from this's records
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    // Initialize values, so we don't change its size so many times
    vtkSmartPointer< vtkLabelVector > distVector = vtkSmartPointer< vtkLabelVector >::New();
	  distVector->FillElements( vectors.size(), 0.0 );

    for ( int j = 0; j < vectors.size(); j++ )
	  {      
      // First, ensure that the records are the same size
      if ( vtkLabelRecord::SafeDownCast( this->GetRecord( 0 ) )->GetVector()->Size() != vectors.at(j)->Size() )
      {
        return dists;
      }
      
      // Initialize the sum to zero
	    double currSum = 0.0;
      for ( int d = 0; d < vtkLabelRecord::SafeDownCast( this->GetRecord( i ) )->GetVector()->Size(); d++ )
	    {
        double currDiff = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) )->GetVector()->GetElement( d ) - vectors.at(j)->GetElement( d );
        currSum += currDiff * currDiff;
	    }
	    
      // Add to the current order record
	    distVector->SetElement( j, sqrt( currSum ) );
	  }

	  // Add the current order record to the vector
    distVector->SetLabel( vtkLabelRecord::SafeDownCast( this->GetRecord( i ) )->GetVector()->GetLabel() );
	  dists.push_back( distVector );
  }

  return dists;
}


// Calculate the record in the buffer that is closest to a particular point
vtkLabelRecord* vtkWorkflowLogRecordBuffer
::ClosestRecord( vtkLabelVector* vector )
{
  if ( vtkLabelRecord::SafeDownCast( this->GetRecord( 0 ) )->GetVector()->Size() != vector->Size() )
  {
    return vtkLabelRecord::New();
  }

  // Calculate the distance to this point
  std::vector< vtkSmartPointer< vtkLabelVector > > vectors( 1, vector );
  std::vector< vtkSmartPointer< vtkLabelVector > > dists = this->Distances( vectors );

  // Now find the closest point
  double minDist = std::numeric_limits< double >::max();
  vtkLabelRecord* minRecord = NULL;  
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    if ( dists.at( i )->GetElement( 0 ) < minDist )
	  {
      minDist = dists.at( i )->GetElement( 0 );
      minRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
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
  vtkLabelRecord* secondRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( 1 ) );
  derivFirstRecord->Copy( firstRecord );
  
  DT = secondRecord->GetTime() - firstRecord->GetTime();
  for( int d = 0; d < firstRecord->GetVector()->Size(); d++ )
  {
    derivFirstRecord->GetVector()->SetElement( d, ( secondRecord->GetVector()->GetElement( d ) - firstRecord->GetVector()->GetElement( d ) ) / DT );
  }

  derivRecordBuffer->AddRecord( derivFirstRecord );


  // Middle (centred difference formula)
  for ( int i = 1; i < this->GetNumRecords() - 1; i++ )
  {
  
    vtkSmartPointer< vtkLabelRecord > derivMiddleRecord = vtkSmartPointer< vtkLabelRecord >::New();
    vtkLabelRecord* middleBeforeRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i - 1 ) );
    vtkLabelRecord* middleRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    vtkLabelRecord* middleAfterRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i + 1 ) );
    derivMiddleRecord->Copy( middleRecord );
  
    DT = middleAfterRecord->GetTime() - middleBeforeRecord->GetTime();
    for( int d = 0; d < middleRecord->GetVector()->Size(); d++ )
    {
      derivMiddleRecord->GetVector()->SetElement( d, ( middleAfterRecord->GetVector()->GetElement( d ) - middleBeforeRecord->GetVector()->GetElement( d ) ) / DT );
    }

    derivRecordBuffer->AddRecord( derivMiddleRecord );
  }


  // Last (backward difference formula)
  vtkSmartPointer< vtkLabelRecord > derivLastRecord = vtkSmartPointer< vtkLabelRecord >::New();
  vtkLabelRecord* lastRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( this->GetNumRecords() - 1 ) );
  vtkLabelRecord* secondlastRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( this->GetNumRecords() - 2 ) );
  derivLastRecord->Copy( lastRecord );
  
  DT = lastRecord->GetTime() - secondlastRecord->GetTime();
  for( int d = 0; d < lastRecord->GetVector()->Size(); d++ )
  {
    derivLastRecord->GetVector()->SetElement( d, ( lastRecord->GetVector()->GetElement( d ) - secondlastRecord->GetVector()->GetElement( d ) ) / DT );
  }

  derivRecordBuffer->AddRecord( derivLastRecord );
  
  // Set this to be the differentiated buffer
  this->Copy( derivRecordBuffer );
  this->Differentiate( order - 1 );
}



void vtkWorkflowLogRecordBuffer
::Integrate( vtkLabelVector* intVector )
{
  // The record log will only hold one record at the end
  int size = vtkLabelRecord::SafeDownCast( this->GetRecord( 0 ) )->GetVector()->Size();
  intVector->FillElements( size, 0.0 );

  // For each time
  for ( int i = 1; i < this->GetNumRecords(); i++ )
  {
	  // Find the time difference
    vtkLabelRecord* currRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    vtkLabelRecord* prevRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i - 1 ) );

    double DT = currRecord->GetTime() - prevRecord->GetTime();

	  // Iterate over all dimensions
    // This is the trapezoidal rule
	  for ( int d = 0; d < size; d++ )
	  {
	    intVector->IncrementElement( d, DT * ( currRecord->GetVector()->GetElement( d ) + prevRecord->GetVector()->GetElement( d ) ) / 2 );
	  }
  }

}


std::vector< vtkSmartPointer< vtkLabelVector > > vtkWorkflowLogRecordBuffer
::LegendreTransformation( int order )
{
  std::vector< vtkSmartPointer< vtkLabelVector > > legendreCoefficientMatrix;

  // The time-shifted record buffer
  vtkSmartPointer< vtkWorkflowLogRecordBuffer > shiftBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();

  // Calculate the time adjustment (need range -1 to 1)
  double startTime = this->GetRecord( 0 )->GetTime();
  double endTime = this->GetCurrentRecord()->GetTime();
  double rangeTime = endTime - startTime;
  
  if ( rangeTime <= 0 )
  {
    return legendreCoefficientMatrix;
  }
  
  // Have to copy records to new buffer because sortedness is maintained on each buffer
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkLabelRecord* currRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    if ( currRecord == NULL )
    {
      continue;
    }
  
    vtkSmartPointer< vtkLabelRecord > currShiftRecord = vtkSmartPointer< vtkLabelRecord >::New();
    currShiftRecord->Copy( currRecord );
    currShiftRecord->SetTime( 2.0 * ( currRecord->GetTime() - startTime ) / rangeTime - 1 ); // ( tmin, tmax ) --> ( -1, 1 )
    shiftBuffer->AddRecord( currShiftRecord );
  }

  // Populate the coefficient matrix
  for ( int o = 0; o <= order; o++ )
  {
	  vtkSmartPointer< vtkWorkflowLogRecordBuffer > unintegratedRecordBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();

    // Multiply the values by the Legendre polynomials
    for ( int i = 0; i < shiftBuffer->GetNumRecords(); i++ )
    {
      vtkLabelRecord* currShiftRecord = vtkLabelRecord::SafeDownCast( shiftBuffer->GetRecord( i ) );
      double legendrePolynomial = LegendrePolynomial( currShiftRecord->GetTime(), o );

	    vtkSmartPointer< vtkLabelRecord > unintegratedRecord = vtkSmartPointer< vtkLabelRecord >::New();

      for ( int d = 0; d < currShiftRecord->GetVector()->Size(); d++ )
	    {	    
        unintegratedRecord->GetVector()->AddElement( currShiftRecord->GetVector()->GetElement( d ) * legendrePolynomial );
	    }

	    unintegratedRecord->SetTime( currShiftRecord->GetTime() );
	    unintegratedRecord->GetVector()->SetLabel( currShiftRecord->GetVector()->GetLabel() );

	    unintegratedRecordBuffer->AddRecord( unintegratedRecord );
    }

	  // Integrate to get the Legendre coefficients for the particular order
    vtkSmartPointer< vtkLabelVector > legendreVector = vtkSmartPointer< vtkLabelVector >::New();
    unintegratedRecordBuffer->Integrate( legendreVector );
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
    // Get the current record
    vtkLabelRecord* currRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );

    // Create a new record    
    vtkSmartPointer< vtkLabelRecord > gaussRecord = vtkSmartPointer< vtkLabelRecord >::New();
    gaussRecord->GetVector()->FillElements( currRecord->GetVector()->Size(), 0.0 );

    // Iterate over all dimensions
	  for ( int d = 0; d < currRecord->GetVector()->Size(); d++ )
	  {
      double weightSum = 0.0;
      double normSum = 0.0;

      // Iterate over all records nearby to the left
	    int j = i;
	    while ( j >= 0 ) // Iterate backward
      {
	      // If too far from "peak" of distribution, the stop - we're just wasting time
        vtkLabelRecord* jthRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( j ) );
	      double normalizedDistance = ( jthRecord->GetTime() - currRecord->GetTime() ) / width;
		    if ( abs( normalizedDistance ) > STDEV_CUTOFF )
		    {
		      break;
		    }
        
        // Calculate the values of the Gaussian distribution at this time
	      double gaussianWeight = exp( - normalizedDistance * normalizedDistance / 2 );
		    // Add the product with the values to function sum
        weightSum = weightSum + jthRecord->GetVector()->GetElement( d ) * gaussianWeight;
		    // Add the values to normSum
		    normSum = normSum + gaussianWeight;

		    j--;
      }

	    // Iterate over all records nearby to the right
	    j = i + 1;
	    while ( j < this->GetNumRecords() ) // Iterate forward
      {
	      // If too far from "peak" of distribution, the stop - we're just wasting time
        vtkLabelRecord* jthRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( j ) );
	      double normalizedDistance = ( jthRecord->GetTime() - currRecord->GetTime() ) / width;
		    if ( abs( normalizedDistance ) > STDEV_CUTOFF )
		    {
		      break;
		    }
        
        // Calculate the values of the Gaussian distribution at this time
	      double gaussianWeight = exp( - normalizedDistance * normalizedDistance / 2 );
		    // Add the product with the values to function sum
        weightSum = weightSum + jthRecord->GetVector()->GetElement( d ) * gaussianWeight;
		    // Add the values to normSum
		    normSum = normSum + gaussianWeight;

		    j++;
      }

	    // Add to the new values
	    gaussRecord->GetVector()->SetElement( d, weightSum / normSum );

	  }

	  // Add the new record vector to the record log
	  gaussRecord->SetTime( currRecord->GetTime() );
	  gaussRecord->GetVector()->SetLabel( currRecord->GetVector()->GetLabel() );
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
    vtkLabelRecord* currRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    vtkSmartPointer< vtkLabelRecord > currLegendreRecord = vtkSmartPointer< vtkLabelRecord >::New();
    currLegendreRecord->GetVector()->FillElements( currRecord->GetVector()->Size() * ( order + 1 ), 0.0 );

    // Calculate the record log to include
    vtkSmartPointer< vtkWorkflowLogRecordBuffer > rangeBuffer = vtkSmartPointer< vtkWorkflowLogRecordBuffer >::New();
    paddedBuffer->GetRange( i, i + window, rangeBuffer );
	  
    std::vector< vtkSmartPointer< vtkLabelVector > > legendreCoefficientMatrix;
    legendreCoefficientMatrix = rangeBuffer->LegendreTransformation( order );

	  // Calculate the Legendre coefficients: 2D -> 1D
	  int count = 0;
	  for ( int o = 0; o <= order; o++ )
    {
      for ( int d = 0; d < currRecord->GetVector()->Size(); d++ )
	    {
        currLegendreRecord->GetVector()->SetElement( count, legendreCoefficientMatrix.at( o )->GetElement( d ) );
		    count++;
	    }
    }

	  // New value record to add to the record log
    currLegendreRecord->SetTime( currRecord->GetTime() );
	  currLegendreRecord->GetVector()->SetLabel( currRecord->GetVector()->GetLabel() );
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
  vtkLabelRecord* currRecord = vtkLabelRecord::SafeDownCast( this->GetCurrentRecord() );
  vnl_matrix<double>* covariance = new vnl_matrix<double>( currRecord->GetVector()->Size(), currRecord->GetVector()->Size() );
  covariance->fill( 0.0 );

  // Determine the mean, subtract the mean from each record
  vtkSmartPointer< vtkLabelVector > meanVector = vtkSmartPointer< vtkLabelVector >::New();
  this->Mean( meanVector );

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
	    zeroMeanRecord->GetVector()->IncrementElement( d, meanVector->GetElement( d ) );
	  }

	  zeroMeanBuffer->AddRecord( zeroMeanRecord );
  }

  // Iterate over all times
  for ( int i = 0; i < zeroMeanBuffer->GetNumRecords(); i++ )
	{
    vtkLabelRecord* currZeroMeanRecord = vtkLabelRecord::SafeDownCast( zeroMeanBuffer->GetRecord( i ) );
    // Pick two dimensions, and find their covariance
    for ( int d1 = 0; d1 < currRecord->GetVector()->Size(); d1++ )
    {
      for ( int d2 = 0; d2 < currRecord->GetVector()->Size(); d2++ )
	    {        
        // Division by number of records is distributed
	      covariance->put( d1, d2, covariance->get( d1, d2 ) + currZeroMeanRecord->GetVector()->GetElement( d1 ) * currZeroMeanRecord->GetVector()->GetElement( d2 ) / zeroMeanBuffer->GetNumRecords() );
	    }
	  }
  }

  return covariance;
}



std::vector< vtkSmartPointer< vtkLabelVector > > vtkWorkflowLogRecordBuffer
::CalculatePCA( int numComp )
{
  std::vector< vtkSmartPointer< vtkLabelVector > > prinComps;

  // Calculate the covariance matrix
  vnl_matrix<double>* covariance = this->CovarianceMatrix();

  //Calculate the eigenvectors of the covariance matrix
  vnl_matrix<double> eigenvectors( covariance->rows(), covariance->cols(), 0.0 );
  vnl_vector<double> eigenvalues( covariance->rows(), 0.0 );
  vnl_symmetric_eigensystem_compute( *covariance, eigenvectors, eigenvalues );
  // Note: eigenvectors are ordered in increasing eigenvalue ( 0 = smallest, end = biggest )

  // Prevent more prinicipal components than original dimensions
  if ( numComp > eigenvectors.cols() )
  {
    numComp = eigenvectors.cols();
  }

  for ( int i = eigenvectors.cols() - 1; i > eigenvectors.cols() - 1 - numComp; i-- )
  {
    vtkSmartPointer< vtkLabelVector > currPrinComp = vtkSmartPointer< vtkLabelVector >::New();
    currPrinComp->FillElements( eigenvectors.rows(), 0.0 );
    
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
::TransformPCA( std::vector< vtkSmartPointer< vtkLabelVector > > prinComps, vtkLabelVector* meanVector )
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
    pcaTransformRecord->GetVector()->FillElements( prinComps.size(), 0.0 );
    
    // Initialize the components of the transformed time record to be zero
	  for ( int o = 0; o < prinComps.size(); o++ )
	  {
	    // Iterate over all dimensions, and perform the transformation (i.e. vector multiplication)
      for ( int d = 0; d < currRecord->GetVector()->Size(); d++ )
	    {
        pcaTransformRecord->GetVector()->IncrementElement( o, ( currRecord->GetVector()->GetElement( d ) - meanVector->GetElement( d ) ) * prinComps.at(o)->GetElement( d ) );
	    }
	  }

    // Copy the transformed values into the current record, which is held onto by the current buffer
    currRecord->GetVector()->SetAllValues( pcaTransformRecord->GetVector()->GetAllValues() );
  }

}



std::vector< vtkSmartPointer< vtkLabelVector > > vtkWorkflowLogRecordBuffer
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
      vtkSmartPointer< vtkLabelVector > currCentroid = vtkSmartPointer< vtkLabelVector >::New();
      this->Mean( currCentroid );
	    currCentroid->SetLabel( k );
	    centroids.push_back( currCentroid );
	    continue;
	  }

    vtkSmartPointer< vtkLabelVector > nextCentroid = vtkSmartPointer< vtkLabelVector >::New();
    this->FindNextCentroid( centroids, nextCentroid );
	  centroids.push_back( nextCentroid );

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




void vtkWorkflowLogRecordBuffer
::FindNextCentroid( std::vector< vtkSmartPointer< vtkLabelVector > > centroids, vtkLabelVector* nextCentroid )
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
  nextCentroid->SetAllValues( vtkLabelRecord::SafeDownCast( this->GetRecord( candidateRecord ) )->GetVector()->GetAllValues() );
  nextCentroid->SetLabel( centroids.size() );
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
::FindEmptyClusters( std::vector< vtkSmartPointer< vtkLabelVector > > centroids, std::vector< int > membership )
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
    if ( emptyVector.at( c ) )
	  {
      return true;
	  }
  }
 
  return false;
}



std::vector< vtkSmartPointer< vtkLabelVector > > vtkWorkflowLogRecordBuffer
::MoveEmptyClusters( std::vector< vtkSmartPointer< vtkLabelVector > > centroids, std::vector< bool > emptyVector )
{
  // Remove any emptyness
  for ( int c = 0; c < centroids.size(); c++ )
  {
    if ( emptyVector.at(c) == false )
	  {
	    continue;
	  }

    vtkSmartPointer< vtkLabelVector > nextCentroid = vtkSmartPointer< vtkLabelVector >::New();
    this->FindNextCentroid( centroids, nextCentroid );
	  centroids.at(c) = nextCentroid;
  }

  return centroids;
}


std::vector< int > vtkWorkflowLogRecordBuffer
::ReassignMembership( std::vector< vtkSmartPointer< vtkLabelVector > > centroids )
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



std::vector< vtkSmartPointer< vtkLabelVector > > vtkWorkflowLogRecordBuffer
::RecalculateCentroids( std::vector< int > membership, int numClusters )
{

  // For each cluster, have an order record and a count
  std::vector< vtkSmartPointer< vtkLabelVector > > centroids;
  std::vector< int > memberCount( numClusters, 0 );

  // Initialize the list of centroids
  for ( int c = 0; c < numClusters; c++ )
  {
    vtkSmartPointer< vtkLabelVector > currCentroid = vtkSmartPointer< vtkLabelVector >::New();
    currCentroid->FillElements( vtkLabelRecord::SafeDownCast( this->GetRecord( 0 ) )->GetVector()->Size(), 0.0 );
	  centroids.push_back( currCentroid );
  }

  // Iterate over all time
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    // For each dimension
    vtkLabelRecord* currRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    for ( int d = 0; d < currRecord->GetVector()->Size(); d++ )
	  {
	    centroids.at( membership.at(i) )->IncrementElement( d, currRecord->GetVector()->GetElement( d ) );
	  }
    
    memberCount.at( membership.at(i) )++;
  }

  // Divide by the number of records in the cluster to get the mean
  for ( int c = 0; c < numClusters; c++ )
  {
    // For each dimension
    for ( int d = 0; d < centroids.at( c )->Size(); d++ )
	  {
	    centroids.at( c )->SetElement( d, centroids.at( c )->GetElement( d ) / memberCount.at( c ) );
	  }
	  centroids.at(c)->SetLabel( c );
  }

  return centroids;
}



void vtkWorkflowLogRecordBuffer
::fwdkmeansTransform( std::vector< vtkSmartPointer< vtkLabelVector > > centroids )
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
    currRecord->GetVector()->SetAllValues( currMembership );
  }

}


void vtkWorkflowLogRecordBuffer
::GetLabelledRange( std::vector< std::string > labels, vtkWorkflowLogRecordBuffer* rangeRecordBuffer )
{
  rangeRecordBuffer->Clear();

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

}


std::vector< vtkSmartPointer< vtkMarkovVector > > vtkWorkflowLogRecordBuffer
::ToMarkovVectors()
{
  std::vector< vtkSmartPointer< vtkMarkovVector > > markovVectors;

  // We will assume that: label -> state, values[0] -> symbol
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkSmartPointer< vtkMarkovVector > currMarkovVector = vtkSmartPointer< vtkMarkovVector >::New();
    vtkLabelRecord* currRecord = vtkLabelRecord::SafeDownCast( this->GetRecord( i ) );
    currMarkovVector->SetState( currRecord->GetVector()->GetLabel() );
	  currMarkovVector->SetSymbol( currRecord->GetVector()->GetElement( 0 ) );
	  markovVectors.push_back( currMarkovVector );
  }

  return markovVectors;
}