

#include "vtkRecordBuffer.h"

vtkStandardNewMacro( vtkRecordBuffer );



// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLTransformBufferNode
::vtkMRMLTransformBufferNode()
{
  // No need to initialize the vectors
}


vtkMRMLTransformBufferNode
::~vtkMRMLTransformBufferNode()
{
  this->transforms.clear(); // Clear function automatically deconstructs all objects in the vector
  this->messages.clear();
}



// Records ------------------------------------------------------------------------


void vtkRecordBuffer
::AddRecord( vtkLabelRecord* newRecord )
{
  // If the proposed new record is not of the same length as other records, then it doesn't belong in the same buffer
  if ( this->GetNumRecords() > 0 && newRecord->Size() != this->GetCurrentRecord()->Size() )
  {
    return;
  }

  // Ensure that we put it into sorted order (usually it will go at the end)
  if ( this->GetNumRecords() < 1 )
  {
    this->records.push_back( newRecord );
	return;
  }
  if ( newRecord->GetTime() > this->GetCurrentRecord()->GetTime() )
  {
    this->records.push_back( newRecord );
	return;
  }
  if ( newRecord->GetTime() < this->GetRecordAt(0)->GetTime() )
  {
    this->records.insert( records.begin() + 0, newRecord );
	return;
  }

  // TODO: Use binary search
  for ( int i = this->GetNumRecords() - 1; i >= 0; i-- )
  {
    if ( newRecord->GetTime() > this->GetRecordAt(i)->GetTime() )
	{
      this->records.insert( records.begin() + i + 1, newRecord );
	  return;
	}
  }

}


void vtkRecordBuffer
::SetRecordAt( int index, vtkLabelRecord* newRecord )
{
  if ( newRecord->Size() != this->GetCurrentRecord()->Size() )
  {
    return;
  }

  this->records.at(index) = newRecord;
}


void vtkRecordBuffer
::RemoveRecordAt( int index )
{
  this->records.erase( records.begin() + index );
}


vtkLabelRecord* vtkRecordBuffer
::GetRecordAt( int index )
{
  return this->records.at( index );
}


vtkLabelRecord* vtkRecordBuffer
::GetCurrentRecord()
{
  return this->records.at( this->GetNumRecords() - 1 );
}


vtkLabelRecord* vtkRecordBuffer
::GetRecordAtTime( double time )
{
  if ( this->GetNumRecords() == 0 )
  {
    return NULL;
  }
  if ( time < this->GetRecordAt(0)->GetTime() )
  {
    return this->GetRecordAt( 0 );
  }
  if ( time > this->GetCurrentRecord()->GetTime() )
  {
    return this->GetCurrentRecord();
  }

  // TODO: Use binary search
  int candidate1, candidate2;
  for ( int i = this->GetNumRecords() - 1; i >= 0; i-- )
  {
    if ( time > this->GetRecordAt(i)->GetTime() )
	{
	  candidate1 = i;
	  candidate2 = i + 1;
	  break;
	}
  }
  	  
  if ( abs ( time - this->GetRecordAt(candidate1)->GetTime() ) < abs ( time - this->GetRecordAt(candidate2)->GetTime() ) )
  {
    return this->GetRecordAt( candidate1 );
  }
  else
  {
    return this->GetRecordAt( candidate2 );
  }

}


int vtkRecordBuffer
::GetNumRecords()
{
  return this->records.size();
}


void vtkMRMLTransformBufferNode
::Clear()
{
  // Note that the clear function calls the destructor for all of the objects in the vector
  this->records.clear();
}


std::string vtkRecordBuffer
::GetName()
{
  return this->name;
}


void vtkRecordBuffer
::SetName( std::string newName )
{
  this->name = newName;
}



vtkMRMLTransformBufferNode* vtkRecordBuffer
::ToTransformBufferNode()
{
  vtkMRMLTransformBufferNode* transformBufferNode;

  // Make sure that this is convertible into matrix form
  if ( this->GetCurrentRecord->Size() != vtkTrackingRecord::TRACKINGRECORD_SIZE )
  {
    return;
  }

  for ( int i = 0; i < this->GetNumRecords; i++ )
  {
    vtkTransformRecord* transformRecord = ( (vtkTrackingRecord*) this->GetRecordAt(i) )->ToTransformRecord();
    transformBufferNode->AddTransform( transformRecord );
  }

  // We must convet the labels into messages
  std::string prevLabel = "";
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    if ( this->GetRecordAt(i)->GetLabel().compare( prevLabel ) != 0 )
	{
	  vtkMessageRecord* messageRecord = vtkMessageRecord::New();
	  messageRecord->SetTime( this->GetRecordAt(i)->GetTime() );
	  messageRecord->SetName( this->GetRecordAt(i)->GetLabel() );
	  transformBufferNode->AddMessage( messageRecord );
	}
  }

  return transformBuffferNode;
  
}


// Only transforms of one type will be stored here
// This method will take those transform from a transform buffer
void vtkRecordBuffer
::FromTransformBufferNode( vtkMRMLTransformBufferNode* newTransformBufferNode )
{
  this->Clear();

  for ( int i = 0; i < newTransformBufferNode->GetNumTransforms(); i++ )
  {
    vtkTrackingRecord trackingRecord = vtkTrackingRecord::New();
    trackingRecord->FromTransformRecord( newTransformBufferNode->GetTransformAt(i) );
    this->AddRecord( trackingRecord );
  }

  // This is a NumTransforms by NumMessages order algorithm anyway...
  // This will work because the messages are sorted by increased time
  for ( int i = 0; i < newTransformsBufferNode->GetNumMessages(); i++ )
  {
    for ( int j = 0; j < this->GetNumRecords(); j++ )
	{
      if ( this->GetRecordAt(j)->GetTime() > newTransformBufferNode->GetMessageAt(i)->GetTime() )
	  {
        this->GetRecordAt(j)->SetLabel( newTransformBufferNode->GetMessageAt(i)->GetName() );
	  }
	}
  }

}


// In theory this should never be saved, but saving might be useful for debugging, so let's allow it for now
std::string vtkRecordBuffer
::ToXMLString()
{
  std::stringstream xmlstring;
  
  xmlstring << "<WorkflowRecordBuffer>" << std::endl;

  for ( int i = 0; i < this->GetNumTransforms(); i++ )
  {
    xmlstring << this->GetRecordAt(i)->ToXMLString( "log" );
  }

  xmlstring << "</WorkflowRecordBuffer>" << std::endl;

  return xmlstring.str();
}


void vtkRecordBuffer
::FromXMLElement( vtkXMLDataElement* rootElement )
{
  if ( ! rootElement || strcmp( rootElement->GetName(), "WorkflowRecordBuffer" ) != 0 ) 
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

	vtkLabelRecord* newRecord = vtkLabelRecord::New();
	newRecord->FromXMLElement( element, "log" );
	this->AddRecord( newRecord );
   
  }

}



// Methods specific for workflow segmentation -------------------------------------------------------------

vtkRecordBuffer* vtkRecordBuffer
::Trim( int start, int end )
{

  // Include the endpoints
  vtkRecordBuffer* trimRecordBuffer = vtkRecordBuffer::New();
  trimRecordBuffer->Initialize( end - start + 1, this->recordSize );

  vtkLabelRecord* currRecord;
  currRecord.initialize( this->recordSize, 0.0 );

  // Iterate over all tracking records in the current procedure, and add to new
  int count = 0;
  for ( int i = start; i <= end; i++ )
  {
	for ( int d = 0; d < this->recordSize; d++ )
	{
      currRecord.set( d, this->GetRecordAt(i).get(d) );
	}

	currRecord.setTime( this->GetRecordAt(i).getTime() );
	currRecord.setLabel( this->GetRecordAt(i).getLabel() );
    trimRecordBuffer->SetRecord( count, currRecord );
	count++;

  }

  return trimRecordBuffer;

}



vtkRecordBuffer* vtkRecordBuffer
::Concatenate( vtkRecordBuffer* otherRecordBuffer )
{
  // First, deep copy the current record log and the concatenating one, so two record logs do not have access to the same record object
  vtkRecordBuffer* currRecordBufferCopy = this->DeepCopy();
  vtkRecordBuffer* otherRecordBufferCopy = otherRecordBuffer->DeepCopy();

  // Create a new record log and assign the copies to it
  vtkRecordBuffer* catRecordBuffer = vtkRecordBuffer::New();

  // Ensure that the record size are the same
  if ( this->recordSize != otherRecordBuffer->recordSize )
  {
    return currRecordBufferCopy;
  }

  catRecordBuffer->Initialize( this->numRecords + otherRecordBuffer->numRecords, this->recordSize );

  int count = 0;
  for( int i = 0; i < currRecordBufferCopy->numRecords; i++ )
  {
    catRecordBuffer->SetRecord( count, currRecordBufferCopy->records[i] );
	count++;
  }

  for( int i = 0; i < otherRecordBufferCopy->numRecords; i++ )
  {
    catRecordBuffer->SetRecord( count, otherRecordBufferCopy->records[i] );
	count++;
  }

  currRecordBufferCopy->Delete();
  otherRecordBufferCopy->Delete();

  return catRecordBuffer;
}



vtkRecordBuffer* vtkRecordBuffer
::ConcatenateValues( vtkRecordBuffer* otherRecordBuffer )
{
  // Only works if the number of records are the same for both record logs
  if ( this->numRecords != otherRecordBuffer->numRecords )
  {
    return this->DeepCopy();
  }

  // Iterate over all records in both logs and stick them together
  vtkRecordBuffer* catRecordBuffer = vtkRecordBuffer::New();
  catRecordBuffer->Initialize( this->numRecords, this->recordSize + otherRecordBuffer->recordSize );

  vtkLabelRecord* currRecord;
  currRecord.initialize( this->recordSize + otherRecordBuffer->recordSize, 0.0 );

  for ( int i = 0; i < this->numRecords; i++ )
  {
	int count = 0;
    for ( int d = 0; d < this->recordSize; d++ )
	{
      currRecord.set( count, this->GetRecordAt(i).get(d) );
	  count++;
	}
	for ( int d = 0; d < otherRecordBuffer->recordSize; d++ )
	{
	  currRecord.set( count, otherRecordBuffer->GetRecordAt(i).get(d) );
	  count++;
	}

	currRecord.setTime( this->GetRecordAt(i).getTime() );
	currRecord.setLabel( this->GetRecordAt(i).getLabel() );
    catRecordBuffer->SetRecord( i, currRecord );
  }

  return catRecordBuffer;
}


vtkRecordBuffer* vtkRecordBuffer
::ConcatenateValues( ValueRecord record )
{

  // Iterate over all records in log and stick them together
  vtkRecordBuffer* catRecordBuffer = vtkRecordBuffer::New();
  catRecordBuffer->Initialize( this->numRecords, this->recordSize + record.size() );

  vtkLabelRecord* currRecord;
  currRecord.initialize( this->recordSize + record.size(), 0.0 );

  for ( int i = 0; i < this->numRecords; i++ )
  {
    int count = 0;
    for ( int d = 0; d < this->recordSize; d++ )
	{
      currRecord.set( count, this->GetRecordAt(i).get(d) );
	  count++;
	}
	for ( int d = 0; d < record.size(); d++ )
	{
	  currRecord.set( count, record.get(d) );
	  count++;
	}

	currRecord.setTime( this->GetRecordAt(i).getTime() );
	currRecord.setLabel( this->GetRecordAt(i).getLabel() );
    catRecordBuffer->SetRecord( i, currRecord );
  }

  return catRecordBuffer;
}



// Note: Does not add to start, just creates start - must concatenate it
vtkRecordBuffer* vtkRecordBuffer
::PadStart( int window )
{
  // Create a new record log of padded values only
  vtkRecordBuffer* padRecordBuffer = vtkRecordBuffer::New();
  padRecordBuffer->Initialize( window, this->recordSize );

  vtkLabelRecord* padRecord;
  padRecord.initialize( this->recordSize, 0.0 );

  // Find the average time stamp
  // Divide by numRecords - 1 because there are one fewer differences than there are stamps
  double DT = 1.0; // TODO: Get a more intelligible default value (not a magic number)
  if ( numRecords > 1 )
  {
    DT = ( GetRecordAt(numRecords-1).getTime() - GetRecordAt(0).getTime() ) / ( numRecords - 1 );
  }

  // Calculate the values and time stamp
  for ( int i = 0; i < window; i++ )
  {

	for ( int d = 0; d < recordSize; d++ )
	{
	   padRecord.set( d, this->GetRecordAt(0).get(d) );
	}

	padRecord.setTime( this->GetRecordAt(0).getTime() - ( i + 1 ) * DT );
	padRecord.setLabel( this->GetRecordAt(0).getLabel() );
	padRecordBuffer->SetRecord( window - ( i + 1 ), padRecord );

  }

  return padRecordBuffer;
}


ValueRecord vtkRecordBuffer
::Mean()
{
  // The record log will only hold one record at the end
  ValueRecord meanRecord;
  meanRecord.initialize( this->recordSize, 0.0 );

  // For each time
  for ( int i = 0; i < this->numRecords; i++ )
  {
	// Iterate over all dimensions
	for ( int d = 0; d < this->recordSize; d++ )
	{
	  meanRecord.set( d, meanRecord.get( d ) + GetRecordAt(i).get(d) );
	}
  }

  // Divide by the number of records
  for ( int d = 0; d < this->recordSize; d++ )
  {
    meanRecord.set( d, meanRecord.get(d) / numRecords );
  }

  return meanRecord;
}



std::vector<vtkLabelVector*> vtkRecordBuffer
::Distances( vtkRecordBuffer* otherRecLog )
{

   // Create a vector of order records
  vtkLabelVector* distRecord;
  std::vector<vtkLabelVector*> dists ( numRecords, distRecord );  

  // First, ensure that the records are the same size
  if ( this->recordSize != otherRecLog->recordSize )
  {
    return dists;
  }
  
  double currSum;
  double currDiff;

  // Otherwise, calculate the vectors based on distances from this's records
  for ( int i = 0; i < numRecords; i++ )
  {
    // Initialize values, so we don't change its size so many times
	distRecord.initialize( otherRecLog->Size(), 0.0 );

    for ( int j = 0; j < otherRecLog->Size(); j++ )
	{
      
      // Initialize the sum to zero
	  currSum = 0.0;
	  for ( int d = 0; d < recordSize; d++ )
	  {
	    currDiff = this->GetRecordAt(i).get(d) - otherRecLog->GetRecordAt(j).get(d);
        currSum += currDiff * currDiff;
	  }
	  // Add to the current order record
	  distRecord.set( j, sqrt( currSum ) );
	}

	// Add the current order record to the vector
	distRecord.setLabel( i );
	dists[i] = distRecord;

  }

  return dists;

}



std::vector<vtkLabelVector*> vtkRecordBuffer
::Distances( std::vector<ValueRecord> valueRecords )
{
  // Create a vector of order records
  vtkLabelVector* distRecord;
  std::vector<vtkLabelVector*> dists ( numRecords, distRecord );  
  
  double currSum;
  double currDiff;

  // Otherwise, calculate the vectors based on distances from this's records
  for ( int i = 0; i < numRecords; i++ )
  {
    // Initialize values, so we don't change its size so many times
	distRecord.initialize( valueRecords.size(), 0.0 );

    for ( int j = 0; j < valueRecords.size(); j++ )
	{
      
      // First, ensure that the records are the same size
      if ( this->recordSize != valueRecords[j].size() )
      {
        return dists;
      }

      // Initialize the sum to zero
	  currSum = 0.0;
	  for ( int d = 0; d < recordSize; d++ )
	  {
	    currDiff = this->GetRecordAt(i).get(d) - valueRecords[j].get(d);
        currSum += currDiff * currDiff;
	  }
	  // Add to the current order record
	  distRecord.set( j, sqrt( currSum ) );
	}

	// Add the current order record to the vector
	distRecord.setLabel( i );
	dists[i] = distRecord;

  }

  return dists;

}


int vtkRecordBuffer
::ClosestRecord( ValueRecord valueRecord )
{
  if ( this->recordSize != valueRecord.size() )
  {
    return -1;
  }

  // Calculate the distance to this point
  std::vector<ValueRecord> valueRecords ( 1, valueRecord );
  std::vector<vtkLabelVector*> dist = this->Distances( valueRecords );

  // Now find the closest point
  double minDist = dist[0].get( 0 );
  int minRecord = 0;
  for ( int i = 0; i < this->numRecords; i++ )
  {
    if ( dist[i].get( 0 ) < minDist )
	{
      minDist = dist[i].get( 0 );
	  minRecord = i;
	}
  }

  return minRecord;

}




vtkRecordBuffer* vtkRecordBuffer
::Derivative( int order )
{
  // If a derivative of order zero is required, then return a copy of this
  if ( order == 0 )
  {
    return this->DeepCopy();
  }

  // Otherwise, calculate the derivative
  vtkRecordBuffer* derivRecordBuffer = vtkRecordBuffer::New();
  derivRecordBuffer->Initialize( this->numRecords, this->recordSize );

  vtkLabelRecord* derivRecord;
  derivRecord.initialize( this->recordSize, 0.0 );

  double DT;

  // First
  DT = this->GetRecordAt(1).getTime() - this->GetRecordAt(0).getTime();

  for( int d = 0; d < this->recordSize; d++ )
  {
    derivRecord.set( d, ( this->GetRecordAt(1).get(d) - this->GetRecordAt(0).get(d) ) / DT );
  }
	
  derivRecord.setTime( this->GetRecordAt(0).getTime() );
  derivRecord.setLabel( this->GetRecordAt(0).getLabel() );
  derivRecordBuffer->SetRecord( 0, derivRecord );


  // Middle
  for ( int i = 1; i < this->numRecords - 1; i++ )
  {

	DT = this->GetRecordAt(i+1).getTime() - this->GetRecordAt(i-1).getTime();

	for( int d = 0; d < this->recordSize; d++ )
	{
      derivRecord.set( d, ( this->GetRecordAt(i+1).get(d) - this->GetRecordAt(i-1).get(d) ) / DT );
	}
	
	derivRecord.setTime( this->GetRecordAt(i).getTime() );
	derivRecord.setLabel( this->GetRecordAt(i).getLabel() );
	derivRecordBuffer->SetRecord( i, derivRecord );

  }


  // Last
  DT = this->GetRecordAt(numRecords-1).getTime() - this->GetRecordAt(numRecords-2).getTime();

  for( int d = 0; d < this->recordSize; d++ )
  {
    derivRecord.set( d, ( this->GetRecordAt(numRecords-1).get(d) - this->GetRecordAt(numRecords-2).get(d) ) / DT );
  }
	
  derivRecord.setTime( this->GetRecordAt(numRecords-1).getTime() );
  derivRecord.setLabel( this->GetRecordAt(numRecords-1).getLabel() );
  derivRecordBuffer->SetRecord( this->numRecords - 1, derivRecord );

  // Return the order - 1 derivative
  vtkRecordBuffer* derivNextRecordBuffer = derivRecordBuffer->Derivative( order - 1 );
  derivRecordBuffer->Delete();
  return derivNextRecordBuffer;

}



ValueRecord vtkRecordBuffer
::Integrate()
{
  // The record log will only hold one record at the end
  ValueRecord intRecord;
  intRecord.initialize( this->recordSize, 0.0 );

  // For each time
  for ( int i = 1; i < numRecords; i++ )
  {
	// Find the time difference
    double DT = GetRecordAt(i).getTime() - GetRecordAt(i-1).getTime();

	// Iterate over all dimensions
	for ( int d = 0; d < recordSize; d++ )
	{
	  intRecord.set( d, intRecord.get( d ) + DT * ( GetRecordAt(i).get(d) + GetRecordAt(i-1).get(d) ) / 2 );
	}

  }

  return intRecord;

}


std::vector<vtkLabelVector*> vtkRecordBuffer
::LegendreTransformation( int order )
{

  // Create a copy of the current record log
  vtkRecordBuffer* recLogCopy = vtkRecordBuffer::New();
  recLogCopy->Initialize( this->numRecords, this->recordSize );

  vtkLabelVector* legRecord;
  legRecord.initialize( recordSize, 0.0 );
  std::vector<vtkLabelVector*> legVector( order + 1, legRecord );

  vtkLabelRecord* currRecord;

  // Calculate the time adjustment (need range -1 to 1)
  double startTime = GetRecordAt(0).getTime();
  double endTime = GetRecordAt( numRecords - 1 ).getTime();
  double rangeTime = endTime - startTime;
  
  for ( int i = 0; i < numRecords; i++ )
  {
	currRecord = GetRecordAt(i);
	currRecord.setTime( ( currRecord.getTime() - startTime ) * 2.0 / rangeTime - 1 ); // tmin - tmax --> -1 - 1
	recLogCopy->SetRecord( i, currRecord );
  }

  // Create a copy of the record log for each degree of Legendre polynomial
  for ( int o = 0; o <= order; o++ )
  {
	vtkRecordBuffer* orderRecLogCopy = vtkRecordBuffer::New();
    orderRecLogCopy->Initialize( this->numRecords, this->recordSize );
	currRecord.initialize( this->recordSize, 0.0 );

    // Multiply the values by the Legendre polynomials
    for ( int i = 0; i < numRecords; i++ )
    {
	  double legPoly = LegendrePolynomial( recLogCopy->GetRecordAt(i).getTime(), o );

      for ( int d = 0; d < recordSize; d++ )
	  {	    
        currRecord.set( d, recLogCopy->GetRecordAt(i).get(d) * legPoly );
	  }

	  currRecord.setTime( recLogCopy->GetRecordAt(i).getTime() );
	  currRecord.setLabel( recLogCopy->GetRecordAt(i).getLabel() );

	  orderRecLogCopy->SetRecord( i, currRecord );

    }

	// Integrate to get the Legendre coefficients for the particular order
	legVector[o].values = orderRecLogCopy->Integrate().values;
	legVector[o].setLabel( o );

	orderRecLogCopy->Delete();

  }

  recLogCopy->Delete();

  return legVector;

}



double vtkRecordBuffer
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



vtkRecordBuffer* vtkRecordBuffer
::GaussianFilter( double width )
{
  // Assume a Gaussian filter
  vtkRecordBuffer* gaussRecordBuffer = vtkRecordBuffer::New();
  gaussRecordBuffer->Initialize( this->numRecords, this->recordSize );

  vtkLabelRecord* gaussRecord;

  // For each record
  for ( int i = 0; i < numRecords; i++ )
  {
    // Create a new record    
	gaussRecord.initialize( recordSize, 0.0 );

    // Iterate over all dimensions
	for ( int d = 0; d < recordSize; d++ )
	{
      double weightSum = 0;
      double normSum = 0;
	  double gaussWeight;
	  double normDist;

      // Iterate over all records nearby
	  int j = i;
	  while ( j >= 0 ) // Iterate backward
      {
	    // If too far from "peak" of distribution, the stop - we're just wasting time
	    normDist = ( GetRecordAt(j).getTime() - GetRecordAt(i).getTime() ) / width;
		if ( abs( normDist ) > STDEV_CUTOFF )
		{
		  break;
		}

        // Calculate the values of the Gaussian distribution at this time
	    gaussWeight = exp( - normDist * normDist / 2 );
		// Add the product with the values to function sum
        weightSum = weightSum + GetRecordAt(j).get(d) * gaussWeight;
		// Add the values to normSum
		normSum = normSum + gaussWeight;

		j--;
      }

	  // Iterate over all records nearby
	  j = i + 1;
	  while ( j < numRecords ) // Iterate forward
      {
	    // If too far from "peak" of distribution, the stop - we're just wasting time
	    normDist = ( GetRecordAt(j).getTime() - GetRecordAt(i).getTime() ) / width;
		if ( abs( normDist ) > STDEV_CUTOFF )
		{
		  break;
		}

        // Calculate the values of the Gaussian distribution at this time
	    gaussWeight = exp( - normDist * normDist / 2 );
		// Add the product with the values to function sum
        weightSum = weightSum + GetRecordAt(j).get(d) * gaussWeight;
		// Add the values to normSum
		normSum = normSum + gaussWeight;

		j++;
      }

	  // Add to the new values
	  gaussRecord.set( d, weightSum / normSum );

	}

	// Add the new record vector to the record log
	gaussRecord.setTime( GetRecordAt(i).getTime() );
	gaussRecord.setLabel( GetRecordAt(i).getLabel() );
    gaussRecordBuffer->SetRecord( i, gaussRecord );

  }

  return gaussRecordBuffer;
}



vtkRecordBuffer* vtkRecordBuffer
::OrthogonalTransformation( int window, int order )
{
  // Pad the RecordBuffer with values at the beginning
  vtkRecordBuffer* padRecordBuffer = this->PadStart( window );
  vtkRecordBuffer* padCatRecordBuffer = padRecordBuffer->Concatenate( this );

  // Create a new record log with the orthogonally transformed data
  vtkRecordBuffer* orthRecordBuffer = vtkRecordBuffer::New();
  orthRecordBuffer->Initialize( this->numRecords, this->recordSize * ( order + 1 ) );

  vtkLabelRecord* currLegRecord;
  currLegRecord.initialize( this->recordSize * ( order + 1 ), 0.0 );

  // Iterate over all data, and calculate Legendre expansion coefficients
  for ( int i = 0; i < this->numRecords; i++ )
  {
    // Calculate the record log to include
    vtkRecordBuffer* trimRecordBuffer = padCatRecordBuffer->Trim( i, i + window );
	
	// Create a new matrix to which the Legendre coefficients will be assigned
	std::vector<vtkLabelVector*> legCoeffMatrix = trimRecordBuffer->LegendreTransformation( order );

	trimRecordBuffer->Delete();

	// Calculate the Legendre coefficients: 2D -> 1D
	int count = 0;
	for ( int o = 0; o <= order; o++ )
    {
      for ( int d = 0; d < recordSize; d++ )
	  {
        currLegRecord.set( count, legCoeffMatrix[o].get(d) );
		count++;
	  }
    }

	// New value record to add to the record log
    currLegRecord.setTime( GetRecordAt(i).getTime() );
	currLegRecord.setLabel( GetRecordAt(i).getLabel() );
	orthRecordBuffer->SetRecord( i, currLegRecord );

  }

  padRecordBuffer->Delete();
  padCatRecordBuffer->Delete();

  return orthRecordBuffer;

}


vnl_matrix<double>* vtkRecordBuffer
::CovarianceMatrix()
{
  // Copy the current record log
  vtkRecordBuffer* covRecLog = vtkRecordBuffer::New();
  covRecLog->Initialize( this->numRecords, this->recordSize );

  // Construct a recordSize by recordSize vnl matrix
  vnl_matrix<double> *cov = new vnl_matrix<double>( recordSize, recordSize );
  cov->fill( 0.0 );

  // Determine the mean, subtract the mean from each record
  ValueRecord meanRecord = this->Mean();
  vtkLabelRecord* currRecord;
  currRecord.initialize( this->recordSize, 0.0 );

  for ( int i = 0; i < this->numRecords; i++ )
  {
    for( int d = 0; d < this->recordSize; d++ )
	{
	  currRecord.set( d, this->GetRecordAt(i).get(d) - meanRecord.get(d) );
	}

	currRecord.setTime( this->GetRecordAt(i).getTime() );
	currRecord.setLabel( this->GetRecordAt(i).getLabel() );

	covRecLog->SetRecord( i, currRecord );
  }

  // Pick two dimensions, and find their covariance
  for ( int d1 = 0; d1 < this->recordSize; d1++ )
  {
    for ( int d2 = 0; d2 < this->recordSize; d2++ )
	{
	  // Iterate over all times
	  for ( int i = 0; i < this->numRecords; i++ )
	  {
	    cov->put( d1, d2, cov->get( d1, d2 ) + covRecLog->GetRecordAt(i).get(d1) * covRecLog->GetRecordAt(i).get(d2) );
	  }
	  // Divide by the number of records
	  cov->put( d1, d2, cov->get( d1, d2 ) / this->numRecords );
	}
  }

  covRecLog->Delete();

  return cov;

}



std::vector<vtkLabelVector*> vtkRecordBuffer
::CalculatePCA( int numComp )
{
  // Calculate the covariance matrix
  vnl_matrix<double>* cov = this->CovarianceMatrix();

  //Calculate the eigenvectors of the covariance matrix
  vnl_matrix<double> eigenvectors( cov->rows(), cov->cols(), 0.0 );
  vnl_vector<double> eigenvalues( cov->rows(), 0.0 );
  vnl_symmetric_eigensystem_compute( *cov, eigenvectors, eigenvalues );
  // Note: eigenvectors are ordered in increasing eigenvalue ( 0 = smallest, end = biggest )

  // Grab only the most important eigenvectors
  vtkLabelVector* currPrinComp;
  currPrinComp.initialize( this->recordSize, 0.0 );

  std::vector<vtkLabelVector*> prinComps ( numComp, currPrinComp );

  // TODO: Prevent more prinicipal components than original dimensions
  for ( int i = eigenvectors.cols() - 1; i > eigenvectors.cols() - 1 - numComp; i-- )
  {
    
    for ( int d = 0; d < this->recordSize; d++ )
	{
	  currPrinComp.set( d, eigenvectors.get( d, i ) );
	}

	currPrinComp.setLabel( eigenvectors.cols() - 1 - i );
	prinComps[ eigenvectors.cols() - 1 - i ] = currPrinComp;

  }

  return prinComps;

}



vtkRecordBuffer* vtkRecordBuffer
::TransformPCA( std::vector<vtkLabelVector*> prinComps, ValueRecord mean )
{
  // Record log with the PCA transformed data
  vtkRecordBuffer* pcaRecLog = vtkRecordBuffer::New();

  // Iterate over all time stamps
  for( int i = 0; i < numRecords; i++ )
  {
    // Create a vtkLabelRecord* for the transformed record log
    vtkLabelRecord* transRecord;

    // Initialize the components of the transformed time record to be zero
	for ( int o = 0; o < prinComps.size(); o++ )
	{
      transRecord.add( 0.0 );
	  
	  // Iterate over all dimensions, and perform the transformation (ie vector multiplcation)
      for ( int d = 0; d < recordSize; d++ )
	  {
        transRecord.set( o, transRecord.get(o) + ( GetRecordAt(i).get(d) - mean.get(d) ) * prinComps[o].get(d) );
	  }
	}

    // Add the time record to the new transformed record log
    transRecord.setTime( GetRecordAt(i).getTime() );
	transRecord.setLabel( GetRecordAt(i).getLabel() );
    pcaRecLog->AddRecord( transRecord );

  }

  return pcaRecLog;

}



std::vector<vtkLabelVector*> vtkRecordBuffer
::fwdkmeans( int numClusters )
{
  // Create a new vector of centroids
  std::vector<vtkLabelVector*> centroids;

  // A vector of cluster memberships
  std::vector<int> membership;
  for ( int i = 0; i < numRecords; i++ )
  {
    membership.push_back( 0 );
  }

  // Iterate until all of the clusters have been added
  for ( int k = 0; k < numClusters; k++ )
  {

	// Use closest point to the mean of all points for the first centroid
    if ( k == 0 )
	{
	  // An order record for the current cluster
      vtkLabelVector* currCentroid;
      currCentroid.values = this->Mean().values;
	  // Add the current centroid to the vector of centroids
	  currCentroid.setLabel( k );
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
	  std::vector<bool> emptyVector = this->FindEmptyClusters( centroids, membership );
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




vtkLabelVector* vtkRecordBuffer
::FindNextCentroid( std::vector<vtkLabelVector*> centroids )
{

  // Find the record farthest from any centroid
  // Tricky way to cast vector of vtkLabelVector* to vector of ValeuRecord
  std::vector<vtkLabelVector*> centDist = this->Distances( std::vector<ValueRecord>( centroids.begin(), centroids.end() ) );
	
  int candidateRecord = 0;
  double candidateDistance = 0;

  for ( int i = 0; i < numRecords; i++ )
  {
    double currMinDist = centDist[i].get(0);
    // Minimum for each point
    for ( int c = 0; c < centroids.size(); c++ )
	{
      if ( centDist[i].get(c) < currMinDist )
	  {
        currMinDist = centDist[i].get(c);
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
  vtkLabelVector* currCentroid;
  currCentroid.values = this->records[candidateRecord].values;
  currCentroid.setLabel( centroids.size() );

  return currCentroid;

}



bool vtkRecordBuffer
::MembershipChanged( std::vector<int> oldMembership, std::vector<int> newMembership )
{

  for ( int i = 0; i < oldMembership.size(); i++ )
  {
    if ( oldMembership[i] != newMembership[i] )
	{
      return true;
	}
  }

  return false;
}



std::vector<bool> vtkRecordBuffer
::FindEmptyClusters( std::vector<vtkLabelVector*> centroids, std::vector<int> membership )
{
  std::vector<bool> emptyVector;

  // Calculate the empty clusters
  for ( int c = 0; c < centroids.size(); c++ )
  {
    emptyVector.push_back(true);
  }
  for ( int i = 0; i < numRecords; i++ )
  {
    emptyVector[ membership[i] ] = false;
  }

  return emptyVector;
}



bool vtkRecordBuffer
::HasEmptyClusters( std::vector<bool> emptyVector )
{
  for ( int c = 0; c < emptyVector.size(); c++ )
  {
    if ( emptyVector[c] )
	{
      return true;
	}
  }
 
  return false;
}



std::vector<vtkLabelVector*> vtkRecordBuffer
::MoveEmptyClusters( std::vector<vtkLabelVector*> centroids, std::vector<bool> emptyVector )
{

  // Remove any emptyness
  for ( int c = 0; c < centroids.size(); c++ )
  {
    if ( emptyVector[c] == false )
	{
	  continue;
	}

    centroids[c] = this->FindNextCentroid( ( centroids ) );

  }

  return centroids;

}


std::vector<int> vtkRecordBuffer
::ReassignMembership( std::vector<vtkLabelVector*> centroids )
{

	
  // Find the record farthest from any centroid
  // Tricky way to cast vector of vtkLabelVector* to vector of ValeuRecord
  std::vector<vtkLabelVector*> centDist = this->Distances( std::vector<ValueRecord>( centroids.begin(), centroids.end() ) );
  
  std::vector<int> membership;

  for ( int i = 0; i < numRecords; i++ )
  {
    double currMinDist = centDist[i].get(0);
	int currMinCentroid = 0;
    // Minimum for each point
    for ( int c = 0; c < centroids.size(); c++ )
	{
      if ( centDist[i].get(c) < currMinDist )
	  {
        currMinDist = centDist[i].get(c);
		currMinCentroid = c;
	  }
	}
    membership.push_back( currMinCentroid );
  }
	
  return membership;
}



std::vector<vtkLabelVector*> vtkRecordBuffer
::RecalculateCentroids( std::vector<int> membership, int numClusters )
{

  // For each cluster, have an order record and a count
  vtkLabelVector* initCentroid;
  initCentroid.initialize( this->recordSize, 0.0 );
  std::vector<vtkLabelVector*> centroids ( numClusters, initCentroid );
  std::vector<int> memberCount ( numClusters, 0 );;

  // Iterate over all time
  for ( int i = 0; i < numRecords; i++ )
  {
    // For each dimension
    for ( int d = 0; d < recordSize; d++ )
	{
	  centroids[ membership[i] ].set( d, centroids[ membership[i] ].get(d) + GetRecordAt(i).get(d) );
	}
    memberCount[ membership[i] ] = memberCount[ membership[i] ] + 1;
  }

  // Divide by the number of records in the cluster to get the mean
  for ( int c = 0; c < numClusters; c++ )
  {
    // For each dimension
    for ( int d = 0; d < recordSize; d++ )
	{
	  centroids[c].set( d, centroids[c].get(d) / memberCount[c] );
	}
	centroids[c].setLabel( c );
  }

  return centroids;

}



vtkRecordBuffer* vtkRecordBuffer
::fwdkmeansTransform( std::vector<vtkLabelVector*> centroids )
{
  // Use the reassign membership function to calculate closest centroids
  std::vector<int> membership = this->ReassignMembership( centroids );

  // Create a copy with the record values replaced by the membership label
  vtkRecordBuffer* clusterRecordBuffer = vtkRecordBuffer::New();

  // Iterate over all tracking records in the current procedure, and add to new
  for ( int i = 0; i < numRecords; i++ )
  {
	vtkLabelRecord* currRecord;
	currRecord.add( membership[i] );
	currRecord.setTime( GetRecordAt(i).getTime() );
	currRecord.setLabel( GetRecordAt(i).getLabel() );
    clusterRecordBuffer->AddRecord( currRecord );
  }
  
  return clusterRecordBuffer;

}


std::vector<vtkRecordBuffer*> vtkRecordBuffer
::GroupRecordsByLabel( int MaxLabel )
{
  std::vector<vtkRecordBuffer*> recordsByTask;

  // For each label, create a new RecordBuffer
  for ( int i = 0; i < MaxLabel; i++ )
  {
	recordsByTask.push_back( vtkRecordBuffer::New() );
  }

  // Iterate over all records
  for ( int i = 0; i < this->Size(); i++ )
  {
    recordsByTask[ this->GetRecordAt(i).getLabel() ]->AddRecord( this->GetRecordAt(i) );
  }

  return recordsByTask;

}


std::vector<MarkovRecord> vtkRecordBuffer
::ToMarkovRecordVector()
{
  std::vector<MarkovRecord> markovRecords;

  // We will assume that: label -> state, values[0] -> symbol
  for ( int i = 0; i < this->Size(); i++ )
  {
    MarkovRecord currMarkovRecord;
    currMarkovRecord.setState( this->GetRecordAt(i).getLabel() );
	currMarkovRecord.setSymbol( this->GetRecordAt(i).get(0) );
	markovRecords.push_back( currMarkovRecord );
  }

  return markovRecords;

}