

#include "vtkRecordBuffer.h"

vtkStandardNewMacro( vtkRecordBuffer );



// Constructors and Destructors --------------------------------------------------------------------

vtkRecordBuffer
::vtkRecordBuffer()
{
  // No need to initialize the vectors
  this->name = "";
}


vtkRecordBuffer
::~vtkRecordBuffer()
{
  vtkDeleteVector( this->records );
  this->name = "";
}


vtkRecordBuffer* vtkRecordBuffer
::DeepCopy()
{
  vtkRecordBuffer* newRecordBuffer = vtkRecordBuffer::New();

  newRecordBuffer->SetName( this->GetName() );
  newRecordBuffer->records = vtkDeepCopyVector( this->records );

  return newRecordBuffer;
}



// Records ------------------------------------------------------------------------


// Use of this function is discouraged - it creates a vtkLabelRecord for each
void vtkRecordBuffer
::Initialize( int newNumRecords, int newRecordSize )
{
  // Create an empty time label record
  vtkLabelRecord* emptyRecord = vtkLabelRecord::New();
  emptyRecord->Initialize( newRecordSize, 0.0 );
  emptyRecord->SetLabel( 0 );
  emptyRecord->SetTime( 0.0 );

  //Set record to be a vector of these empty records
  records = std::vector<vtkLabelRecord*>( newNumRecords, emptyRecord );
}


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
  if ( newRecord->GetTime() >= this->GetCurrentRecord()->GetTime() )
  {
    this->records.push_back( newRecord );
	return;
  }
  if ( newRecord->GetTime() <= this->GetRecordAt(0)->GetTime() )
  {
    this->records.insert( records.begin() + 0, newRecord );
	return;
  }

  // Record is probably near the end - this is more efficient than binary search
  for ( int i = this->GetNumRecords() - 1; i >= 0; i-- )
  {
    if ( newRecord->GetTime() >= this->GetRecordAt(i)->GetTime() )
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

  // Record is probably near the end - this is more efficient than binary search
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


void vtkRecordBuffer
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
	vtkMRMLTransformBufferNode* transformBufferNode = vtkMRMLTransformBufferNode::New();

  // Make sure that this is convertible into matrix form
  if ( this->GetCurrentRecord()->Size() != vtkTrackingRecord::TRACKINGRECORD_SIZE )
  {
    return transformBufferNode;
  }

  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkTransformRecord* transformRecord = ( vtkTrackingRecord::SafeDownCast( this->GetRecordAt(i) ) )->ToTransformRecord();
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
	  prevLabel = this->GetRecordAt(i)->GetLabel();
	}
  }

  return transformBufferNode;
  
}


// Only transforms of one device will be stored here
// This method will take those transform from a transform buffer
void vtkRecordBuffer
::FromTransformBufferNode( vtkMRMLTransformBufferNode* newTransformBufferNode )
{
  this->Clear();

  for ( int i = 0; i < newTransformBufferNode->GetNumTransforms(); i++ )
  {
    vtkTrackingRecord* trackingRecord = vtkTrackingRecord::New();
    trackingRecord->FromTransformRecord( newTransformBufferNode->GetTransformAt(i) );
    this->AddRecord( trackingRecord );
  }

  // This is a NumTransforms by NumMessages order algorithm anyway...
  // This will work because the messages are sorted by increased time
  for ( int i = 0; i < newTransformBufferNode->GetNumMessages(); i++ )
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


// Only keep the messages that are relevant for this tool
// In addition, keep the "End"/"Done" message
void vtkRecordBuffer
::FromTransformBufferNode( vtkMRMLTransformBufferNode* newTransformBufferNode, std::vector<std::string> relevantMessages )
{
  this->Clear();

  for ( int i = 0; i < newTransformBufferNode->GetNumTransforms(); i++ )
  {
    vtkTrackingRecord* trackingRecord = vtkTrackingRecord::New();
    trackingRecord->FromTransformRecord( newTransformBufferNode->GetTransformAt(i) );
    this->AddRecord( trackingRecord );
  }

  // This is a NumTransforms by NumMessages order algorithm anyway...
  // This will work because the messages are sorted by increased time
  for ( int i = 0; i < newTransformBufferNode->GetNumMessages(); i++ )
  {
    // Only accept if message is relevant
    bool relevant = false;
    for ( int j = 0; j < relevantMessages.size(); j++ )
	{
      if ( newTransformBufferNode->GetMessageAt(i)->GetName().compare( relevantMessages.at(j) ) == 0 )
	  {
        relevant = true;
	  }
	}

	// Skip if the message is not relevant or a finishing method
	if ( ! relevant && newTransformBufferNode->GetMessageAt(i)->GetName().compare( "Done" ) != 0 && newTransformBufferNode->GetMessageAt(i)->GetName().compare( "End" ) != 0 )
	{
      continue;
	}

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

  for ( int i = 0; i < this->GetNumRecords(); i++ )
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

  // Iterate over all tracking records in the current procedure, and add to new
  int count = 0;
  for ( int i = start; i <= end; i++ )
  {
    vtkLabelRecord* currRecord = vtkLabelRecord::New();
    currRecord->Initialize( this->GetRecordAt(0)->Size(), 0.0 );

	for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	{
      currRecord->Set( d, this->GetRecordAt(i)->Get(d) );
	}

	currRecord->SetTime( this->GetRecordAt(i)->GetTime() );
	currRecord->SetLabel( this->GetRecordAt(i)->GetLabel() );
    trimRecordBuffer->AddRecord( currRecord );
  }

  return trimRecordBuffer;

}



vtkRecordBuffer* vtkRecordBuffer
::Concatenate( vtkRecordBuffer* otherRecordBuffer )
{

  // Ensure that the record size are the same
  if ( this->GetNumRecords() < 1 )
  {
    return otherRecordBuffer->DeepCopy();
  }
  if ( otherRecordBuffer->GetNumRecords() < 1 )
  {
    return this->DeepCopy();
  }
  if ( this->GetRecordAt(0)->Size() != otherRecordBuffer->GetRecordAt(0)->Size() )
  {
    return NULL;
  }

  // Create a new record log and assign the copies to it
  vtkRecordBuffer* catRecordBuffer = vtkRecordBuffer::New();
   catRecordBuffer->SetName( this->GetName() );

  int count = 0;
  for( int i = 0; i < this->GetNumRecords(); i++ )
  {
    catRecordBuffer->AddRecord( this->GetRecordAt(i)->DeepCopy() );
	count++;
  }

  for( int i = 0; i < otherRecordBuffer->GetNumRecords(); i++ )
  {
    catRecordBuffer->AddRecord( otherRecordBuffer->GetRecordAt(i)->DeepCopy() );
	count++;
  }

  return catRecordBuffer;
}



vtkRecordBuffer* vtkRecordBuffer
::ConcatenateValues( vtkRecordBuffer* otherRecordBuffer )
{
  // Only works if the number of records are the same for both record logs
  if ( this->GetNumRecords() != otherRecordBuffer->GetNumRecords() )
  {
    return this->DeepCopy();
  }

  // Iterate over all records in both logs and stick them together
  vtkRecordBuffer* catRecordBuffer = vtkRecordBuffer::New();
 
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkLabelRecord* currRecord = vtkLabelRecord::New();
    currRecord->Initialize( this->GetRecordAt(0)->Size() + otherRecordBuffer->GetRecordAt(0)->Size(), 0.0 );

	int count = 0;
    for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	{
      currRecord->Set( count, this->GetRecordAt(i)->Get(d) );
	  count++;
	}
	for ( int d = 0; d < otherRecordBuffer->GetRecordAt(0)->Size(); d++ )
	{
	  currRecord->Set( count, otherRecordBuffer->GetRecordAt(i)->Get(d) );
	  count++;
	}

	currRecord->SetTime( this->GetRecordAt(i)->GetTime() );
	currRecord->SetLabel( this->GetRecordAt(i)->GetLabel() );
    catRecordBuffer->AddRecord( currRecord );
  }

  return catRecordBuffer;
}


vtkRecordBuffer* vtkRecordBuffer
::ConcatenateValues( vtkLabelVector* vector )
{

  // Iterate over all records in log and stick them together
  vtkRecordBuffer* catRecordBuffer = vtkRecordBuffer::New();

  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkLabelRecord* currRecord = vtkLabelRecord::New();
    currRecord->Initialize( this->GetRecordAt(0)->Size() + vector->Size(), 0.0 );

    int count = 0;
    for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	{
      currRecord->Set( count, this->GetRecordAt(i)->Get(d) );
	  count++;
	}
	for ( int d = 0; d < vector->Size(); d++ )
	{
	  currRecord->Set( count, vector->Get(d) );
	  count++;
	}

	currRecord->SetTime( this->GetRecordAt(i)->GetTime() );
	currRecord->SetLabel( this->GetRecordAt(i)->GetLabel() );
    catRecordBuffer->AddRecord( currRecord );
  }

  return catRecordBuffer;
}



// Note: Does not add to start, just creates start - must concatenate it
vtkRecordBuffer* vtkRecordBuffer
::PadStart( int window )
{
  // Create a new record log of padded values only
  vtkRecordBuffer* padRecordBuffer = vtkRecordBuffer::New();

  // Find the average time stamp
  // Divide by numRecords - 1 because there are one fewer differences than there are stamps
  double DT = 1.0;
  if ( this->GetNumRecords() > 1 )
  {
    DT = ( this->GetCurrentRecord()->GetTime() - this->GetRecordAt(0)->GetTime() ) / ( this->GetNumRecords() - 1 );
  }

  // Calculate the values and time stamp
  for ( int i = window; i > 0; i-- )
  {
    vtkLabelRecord* currentRecord = this->GetRecordAt(0)->DeepCopy();
	currentRecord->SetTime( this->GetRecordAt(0)->GetTime() - i * DT );
	currentRecord->SetLabel( this->GetRecordAt(0)->GetLabel() );
	padRecordBuffer->AddRecord( currentRecord );
  }

  return padRecordBuffer;
}


vtkLabelVector* vtkRecordBuffer
::Mean()
{
  // The record log will only hold one record at the end
  vtkLabelVector* meanRecord = vtkLabelVector::New();
  meanRecord->Initialize( this->GetRecordAt(0)->Size(), 0.0 );

  // For each time
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
	// Iterate over all dimensions
	for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	{
	  meanRecord->Set( d, meanRecord->Get( d ) + GetRecordAt(i)->Get(d) );
	}
  }

  // Divide by the number of records
  for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
  {
    meanRecord->Set( d, meanRecord->Get(d) / this->GetNumRecords() );
  }

  return meanRecord;
}



std::vector<vtkLabelVector*> vtkRecordBuffer
::Distances( vtkRecordBuffer* otherRecLog )
{

  // Create a vector of order records
  std::vector<vtkLabelVector*> dists;

  // First, ensure that the records are the same size
  if ( this->GetRecordAt(0)->Size() != otherRecLog->GetRecordAt(0)->Size() )
  {
    return dists;
  }

  // Otherwise, calculate the vectors based on distances from this's records
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    // Initialize values, so we don't change its size so many times
    vtkLabelRecord* distRecord = vtkLabelRecord::New();
	distRecord->Initialize( otherRecLog->GetNumRecords(), 0.0 );

    for ( int j = 0; j < otherRecLog->GetNumRecords(); j++ )
	{      
      // Initialize the sum to zero
	  double currSum = 0.0;
	  for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	  {
	    double currDiff = this->GetRecordAt(i)->Get(d) - otherRecLog->GetRecordAt(j)->Get(d);
        currSum += currDiff * currDiff;
	  }
	  // Add to the current order record
	  distRecord->Set( j, sqrt( currSum ) );
	}

	// Add the current order record to the vector
	distRecord->SetLabel( this->GetRecordAt(i)->GetLabel() );
	dists.push_back( distRecord );
  }

  return dists;
}



std::vector<vtkLabelVector*> vtkRecordBuffer
::Distances( std::vector<vtkLabelVector*> vectors )
{
  // Create a vector of order records
  std::vector<vtkLabelVector*> dists;  

  // Otherwise, calculate the vectors based on distances from this's records
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    // Initialize values, so we don't change its size so many times
    vtkLabelVector* distRecord = vtkLabelVector::New();
	distRecord->Initialize( vectors.size(), 0.0 );

    for ( int j = 0; j < vectors.size(); j++ )
	{      
      // First, ensure that the records are the same size
      if ( this->GetRecordAt(0)->Size() != vectors.at(j)->Size() )
      {
        return dists;
      }
      // Initialize the sum to zero
	  double currSum = 0.0;
	  for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	  {
	    double currDiff = this->GetRecordAt(i)->Get(d) - vectors.at(j)->Get(d);
        currSum += currDiff * currDiff;
	  }
	  // Add to the current order record
	  distRecord->Set( j, sqrt( currSum ) );
	}

	// Add the current order record to the vector
	distRecord->SetLabel( this->GetRecordAt(i)->GetLabel() );
	dists.push_back( distRecord );
  }

  return dists;
}


// Calculate the record in the buffer that is closest to a particular point
vtkLabelRecord* vtkRecordBuffer
::ClosestRecord( vtkLabelVector* vector )
{
  if ( this->GetRecordAt(0)->Size() != vector->Size() )
  {
    return vtkLabelRecord::New();
  }

  // Calculate the distance to this point
  std::vector<vtkLabelVector*> vectors( 1, vector );
  std::vector<vtkLabelVector*> dist = this->Distances( vectors );

  // Now find the closest point
  double minDist = dist.at(0)->Get(0);
  vtkLabelRecord* minRecord = this->GetRecordAt(0);
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    if ( dist.at(i)->Get(0) < minDist )
	{
      minDist = dist.at(i)->Get(0);
	  minRecord = this->GetRecordAt(i);
	}
  }

  return minRecord;
}




vtkRecordBuffer* vtkRecordBuffer
::Derivative( int order )
{
  // If a derivative of order zero is required, then return a copy of this
  if ( this->GetNumRecords() < 2 || order == 0 )
  {
    return this->DeepCopy();
  }

  // Otherwise, calculate the derivative
  vtkRecordBuffer* derivRecordBuffer = vtkRecordBuffer::New();

  double DT;

  // First
  vtkLabelRecord* derivRecordFirst = vtkLabelRecord::New();
  derivRecordFirst->Initialize( this->GetRecordAt(0)->Size(), 0.0 );
  DT = this->GetRecordAt(1)->GetTime() - this->GetRecordAt(0)->GetTime();

  for( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
  {
    derivRecordFirst->Set( d, ( this->GetRecordAt(1)->Get(d) - this->GetRecordAt(0)->Get(d) ) / DT );
  }
	
  derivRecordFirst->SetTime( this->GetRecordAt(0)->GetTime() );
  derivRecordFirst->SetLabel( this->GetRecordAt(0)->GetLabel() );
  derivRecordBuffer->AddRecord( derivRecordFirst );


  // Middle
  for ( int i = 1; i < this->GetNumRecords() - 1; i++ )
  {

    vtkLabelRecord* derivRecordMiddle = vtkLabelRecord::New();
    derivRecordMiddle->Initialize( this->GetRecordAt(0)->Size(), 0.0 );
	DT = this->GetRecordAt(i+1)->GetTime() - this->GetRecordAt(i-1)->GetTime();

	for( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	{
      derivRecordMiddle->Set( d, ( this->GetRecordAt(i+1)->Get(d) - this->GetRecordAt(i-1)->Get(d) ) / DT );
	}
	
	derivRecordMiddle->SetTime( this->GetRecordAt(i)->GetTime() );
	derivRecordMiddle->SetLabel( this->GetRecordAt(i)->GetLabel() );
	derivRecordBuffer->AddRecord( derivRecordMiddle );

  }


  // Last
  vtkLabelRecord* derivRecordLast = vtkLabelRecord::New();
  derivRecordLast->Initialize( this->GetRecordAt(0)->Size(), 0.0 );
  DT = this->GetRecordAt( this->GetNumRecords() - 1 )->GetTime() - this->GetRecordAt( this->GetNumRecords() - 2 )->GetTime();

  for( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
  {
    derivRecordLast->Set( d, ( this->GetRecordAt( this->GetNumRecords() - 1 )->Get(d) - this->GetRecordAt( this->GetNumRecords() - 2 )->Get(d) ) / DT );
  }
	
  derivRecordLast->SetTime( this->GetRecordAt( this->GetNumRecords() - 1 )->GetTime() );
  derivRecordLast->SetLabel( this->GetRecordAt( this->GetNumRecords() - 1 )->GetLabel() );
  derivRecordBuffer->AddRecord( derivRecordLast );

  // Return the order - 1 derivative
  vtkRecordBuffer* derivNextRecordBuffer = derivRecordBuffer->Derivative( order - 1 );
  derivRecordBuffer->Delete();
  return derivNextRecordBuffer;

}



vtkLabelVector* vtkRecordBuffer
::Integrate()
{
  // The record log will only hold one record at the end
  vtkLabelVector* intVector = vtkLabelVector::New();
  intVector->Initialize( this->GetRecordAt(0)->Size(), 0.0 );

  // For each time
  for ( int i = 1; i < this->GetNumRecords(); i++ )
  {
	// Find the time difference
    double DT = this->GetRecordAt(i)->GetTime() - this->GetRecordAt(i-1)->GetTime();

	// Iterate over all dimensions
	for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	{
	  intVector->Crement( d, DT * ( this->GetRecordAt(i)->Get(d) + this->GetRecordAt(i-1)->Get(d) ) / 2 );
	}
  }

  return intVector;
}


std::vector<vtkLabelVector*> vtkRecordBuffer
::LegendreTransformation( int order )
{

  // Create a copy of the current record log
  vtkRecordBuffer* shiftRecordBuffer = this->DeepCopy();

  // Calculate the time adjustment (need range -1 to 1)
  double startTime = this->GetRecordAt(0)->GetTime();
  double endTime = this->GetCurrentRecord()->GetTime();
  double rangeTime = endTime - startTime;
  
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    double newTime = 0.0;
    if ( rangeTime > 0 )
	{
      newTime = ( shiftRecordBuffer->GetRecordAt(i)->GetTime() - startTime ) * 2.0 / rangeTime - 1; // tmin - tmax --> -1 - 1
	}
	shiftRecordBuffer->GetRecordAt(i)->SetTime( newTime );
  }

  // Create a copy of the record log for each degree of Legendre polynomial
  std::vector<vtkLabelVector*> legCoeffMatrix;

  for ( int o = 0; o <= order; o++ )
  {
	vtkRecordBuffer* unintRecordBuffer = vtkRecordBuffer::New();

    // Multiply the values by the Legendre polynomials
    for ( int i = 0; i < shiftRecordBuffer->GetNumRecords(); i++ )
    {
      double legPoly = LegendrePolynomial( shiftRecordBuffer->GetRecordAt(i)->GetTime(), o );

	  vtkLabelRecord* unintRecord = vtkLabelRecord::New();

      for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	  {	    
        unintRecord->Add( shiftRecordBuffer->GetRecordAt(i)->Get(d) * legPoly );
	  }

	  unintRecord->SetTime( shiftRecordBuffer->GetRecordAt(i)->GetTime() );
	  unintRecord->SetLabel( shiftRecordBuffer->GetRecordAt(i)->GetLabel() );

	  unintRecordBuffer->AddRecord( unintRecord );
    }

	// Integrate to get the Legendre coefficients for the particular order
	vtkLabelVector* legVector = unintRecordBuffer->Integrate();
	legVector->SetLabel( o );
	legCoeffMatrix.push_back( legVector );

	unintRecordBuffer->Delete();
  }

  shiftRecordBuffer->Delete();
  return legCoeffMatrix;
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

  // For each record
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    // Create a new record    
    vtkLabelRecord* gaussRecord = vtkLabelRecord::New();
	gaussRecord->Initialize( this->GetRecordAt(0)->Size(), 0.0 );

    // Iterate over all dimensions
	for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	{
      double weightSum = 0.0;
      double normSum = 0.0;

      // Iterate over all records nearby
	  int j = i;
	  while ( j >= 0 ) // Iterate backward
      {
	    // If too far from "peak" of distribution, the stop - we're just wasting time
	    double normDist = ( GetRecordAt(j)->GetTime() - GetRecordAt(i)->GetTime() ) / width;
		if ( abs( normDist ) > STDEV_CUTOFF )
		{
		  break;
		}
        // Calculate the values of the Gaussian distribution at this time
	    double gaussWeight = exp( - normDist * normDist / 2 );
		// Add the product with the values to function sum
        weightSum = weightSum + GetRecordAt(j)->Get(d) * gaussWeight;
		// Add the values to normSum
		normSum = normSum + gaussWeight;

		j--;
      }

	  // Iterate over all records nearby
	  j = i + 1;
	  while ( j < this->GetNumRecords() ) // Iterate forward
      {
	    // If too far from "peak" of distribution, the stop - we're just wasting time
	    double normDist = ( GetRecordAt(j)->GetTime() - GetRecordAt(i)->GetTime() ) / width;
		if ( abs( normDist ) > STDEV_CUTOFF )
		{
		  break;
		}
        // Calculate the values of the Gaussian distribution at this time
	    double gaussWeight = exp( - normDist * normDist / 2 );
		// Add the product with the values to function sum
        weightSum = weightSum + GetRecordAt(j)->Get(d) * gaussWeight;
		// Add the values to normSum
		normSum = normSum + gaussWeight;

		j++;
      }

	  // Add to the new values
	  gaussRecord->Set( d, weightSum / normSum );

	}

	// Add the new record vector to the record log
	gaussRecord->SetTime( GetRecordAt(i)->GetTime() );
	gaussRecord->SetLabel( GetRecordAt(i)->GetLabel() );
    gaussRecordBuffer->AddRecord( gaussRecord );

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

  // Iterate over all data, and calculate Legendre expansion coefficients
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkLabelRecord* currLegRecord = vtkLabelRecord::New();
    currLegRecord->Initialize( this->GetRecordAt(0)->Size() * ( order + 1 ), 0.0 );

    // Calculate the record log to include
    vtkRecordBuffer* trimRecordBuffer = padCatRecordBuffer->Trim( i, i + window );
	std::vector<vtkLabelVector*> legCoeffMatrix = trimRecordBuffer->LegendreTransformation( order );
	trimRecordBuffer->Delete();

	// Calculate the Legendre coefficients: 2D -> 1D
	int count = 0;
	for ( int o = 0; o <= order; o++ )
    {
      for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	  {
        currLegRecord->Set( count, legCoeffMatrix.at(o)->Get(d) );
		count++;
	  }
    }

	// New value record to add to the record log
    currLegRecord->SetTime( GetRecordAt(i)->GetTime() );
	currLegRecord->SetLabel( GetRecordAt(i)->GetLabel() );
	orthRecordBuffer->AddRecord( currLegRecord );

    vtkDeleteVector( legCoeffMatrix );
  }

  padRecordBuffer->Delete();
  padCatRecordBuffer->Delete();
  return orthRecordBuffer;
}


vnl_matrix<double>* vtkRecordBuffer
::CovarianceMatrix()
{
  // Copy the current record log
  vtkRecordBuffer* zeroMeanBuffer = vtkRecordBuffer::New();

  // Construct a recordSize by recordSize vnl matrix
  vnl_matrix<double> *cov = new vnl_matrix<double>( this->GetRecordAt(0)->Size(), this->GetRecordAt(0)->Size() );
  cov->fill( 0.0 );

  // Determine the mean, subtract the mean from each record
  vtkLabelVector* meanRecord = this->Mean();

  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkLabelRecord* currRecord = vtkLabelRecord::New();
    currRecord->Initialize( this->GetRecordAt(0)->Size(), 0.0 );

    for( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	{
	  currRecord->Set( d, this->GetRecordAt(i)->Get(d) - meanRecord->Get(d) );
	}

	currRecord->SetTime( this->GetRecordAt(i)->GetTime() );
	currRecord->SetLabel( this->GetRecordAt(i)->GetLabel() );

	zeroMeanBuffer->AddRecord( currRecord );
  }

  // Pick two dimensions, and find their covariance
  for ( int d1 = 0; d1 < this->GetRecordAt(0)->Size(); d1++ )
  {
    for ( int d2 = 0; d2 < this->GetRecordAt(0)->Size(); d2++ )
	{
	  // Iterate over all times
	  for ( int i = 0; i < this->GetNumRecords(); i++ )
	  {
	    cov->put( d1, d2, cov->get( d1, d2 ) + zeroMeanBuffer->GetRecordAt(i)->Get(d1) * zeroMeanBuffer->GetRecordAt(i)->Get(d2) );
	  }
	  // Divide by the number of records
	  cov->put( d1, d2, cov->get( d1, d2 ) / zeroMeanBuffer->GetNumRecords() );
	}
  }

  meanRecord->Delete();
  zeroMeanBuffer->Delete();
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
  std::vector<vtkLabelVector*> prinComps;

  // Prevent more prinicipal components than original dimensions
  if ( numComp > eigenvectors.cols() )
  {
    numComp = eigenvectors.cols();
  }

  for ( int i = eigenvectors.cols() - 1; i > eigenvectors.cols() - 1 - numComp; i-- )
  {
    vtkLabelVector* currPrinComp = vtkLabelVector::New();
    currPrinComp->Initialize( this->GetRecordAt(0)->Size(), 0.0 );
    
    for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	{
	  currPrinComp->Set( d, eigenvectors.get( d, i ) );
	}

    currPrinComp->SetLabel( eigenvectors.cols() - 1 - i );
	prinComps.push_back( currPrinComp );
  }

  return prinComps;
}



vtkRecordBuffer* vtkRecordBuffer
::TransformPCA( std::vector<vtkLabelVector*> prinComps, vtkLabelVector* mean )
{
  // Record log with the PCA transformed data
  vtkRecordBuffer* pcaBuffer = vtkRecordBuffer::New();

  // Iterate over all time stamps
  for( int i = 0; i < this->GetNumRecords(); i++ )
  {
    // Create a vtkLabelRecord* for the transformed record log
    vtkLabelRecord* pcaTransRecord = vtkLabelRecord::New();
	pcaTransRecord->Initialize( prinComps.size(), 0.0 );

    // Initialize the components of the transformed time record to be zero
	for ( int o = 0; o < prinComps.size(); o++ )
	{
	  // Iterate over all dimensions, and perform the transformation (ie vector multiplcation)
      for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	  {
        pcaTransRecord->Set( o, pcaTransRecord->Get(o) + ( GetRecordAt(i)->Get(d) - mean->Get(d) ) * prinComps.at(o)->Get(d) );
	  }
	}

    // Add the time record to the new transformed record log
    pcaTransRecord->SetTime( GetRecordAt(i)->GetTime() );
	pcaTransRecord->SetLabel( GetRecordAt(i)->GetLabel() );
    pcaBuffer->AddRecord( pcaTransRecord );
  }

  return pcaBuffer;
}



std::vector<vtkLabelVector*> vtkRecordBuffer
::fwdkmeans( int numClusters )
{
  // Create a new vector of centroids
  std::vector<vtkLabelVector*> centroids;

  // A vector of cluster memberships
  std::vector<int> membership( this->GetNumRecords(), 0 );

  // Iterate until all of the clusters have been added
  for ( int k = 0; k < numClusters; k++ )
  {

	// Use closest point to the mean of all points for the first centroid
    if ( k == 0 )
	{
	  // An order record for the current cluster
      vtkLabelVector* currCentroid = this->Mean();
	  currCentroid->SetLabel(k);
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
	  vtkDeleteVector( centroids );
      centroids = this->RecalculateCentroids( membership, k + 1 );

	}

  }

  return centroids;

}




vtkLabelVector* vtkRecordBuffer
::FindNextCentroid( std::vector<vtkLabelVector*> centroids )
{
  // Find the record farthest from any centroid
  std::vector<vtkLabelVector*> centDist = this->Distances( centroids );
	
  int candidateRecord = 0;
  double candidateDistance = 0;

  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    double currMinDist = centDist.at(i)->Get(0);
    // Minimum for each point
    for ( int c = 0; c < centroids.size(); c++ )
	{
      if ( centDist.at(i)->Get(c) < currMinDist )
	  {
        currMinDist = centDist.at(i)->Get(c);
	  }
	}
	
	// Maximum of the minimums
    if ( currMinDist > candidateDistance )
    {
      candidateDistance = currMinDist;
      candidateRecord = i;
	}
  }

  vtkDeleteVector( centDist );

  // Create new centroid for candidate
  vtkLabelVector* currCentroid = vtkLabelVector::New();
  currCentroid->SetValues( this->GetRecordAt(candidateRecord)->GetValues() );
  currCentroid->SetLabel( centroids.size() );
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
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    emptyVector.at( membership.at(i) ) = false;
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
    if ( emptyVector.at(c) == false )
	{
	  continue;
	}

	centroids.at(c)->Delete();
    centroids.at(c) = this->FindNextCentroid( centroids );
  }

  return centroids;
}


std::vector<int> vtkRecordBuffer
::ReassignMembership( std::vector<vtkLabelVector*> centroids )
{
  // Find the record farthest from any centroid
  // Tricky way to cast vector of vtkLabelVector* to vector of ValeuRecord
  std::vector<vtkLabelVector*> centDist = this->Distances( centroids );
  
  std::vector<int> membership;

  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    double currMinDist = centDist.at(i)->Get(0);
	int currMinCentroid = 0;
    // Minimum for each point
    for ( int c = 0; c < centroids.size(); c++ )
	{
      if ( centDist.at(i)->Get(c) < currMinDist )
	  {
        currMinDist = centDist.at(i)->Get(c);
		currMinCentroid = c;
	  }
	}
    membership.push_back( currMinCentroid );
  }

  vtkDeleteVector( centDist );
	
  return membership;
}



std::vector<vtkLabelVector*> vtkRecordBuffer
::RecalculateCentroids( std::vector<int> membership, int numClusters )
{

  // For each cluster, have an order record and a count
  std::vector<vtkLabelVector*> centroids;
  std::vector<int> memberCount( numClusters, 0 );

  // Initialize the list of centroids
  for ( int c = 0; c < numClusters; c++ )
  {
    vtkLabelVector* currCentroid = vtkLabelVector::New();
	currCentroid->Initialize( this->GetRecordAt(0)->Size(), 0.0 );
	centroids.push_back( currCentroid );
  }

  // Iterate over all time
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    // For each dimension
    for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	{
	  centroids.at( membership.at(i) )->Crement( d, this->GetRecordAt(i)->Get(d) );
	}
    memberCount.at( membership.at(i) ) = memberCount.at( membership.at(i) ) + 1;
  }

  // Divide by the number of records in the cluster to get the mean
  for ( int c = 0; c < numClusters; c++ )
  {
    // For each dimension
    for ( int d = 0; d < this->GetRecordAt(0)->Size(); d++ )
	{
	  centroids.at(c)->Set( d, centroids.at(c)->Get(d) / memberCount.at(c) );
	}
	centroids.at(c)->SetLabel( c );
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
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkLabelRecord* clusterRecord = vtkLabelRecord::New();
	clusterRecord->Add( membership.at(i) );
	clusterRecord->SetTime( GetRecordAt(i)->GetTime() );
	clusterRecord->SetLabel( GetRecordAt(i)->GetLabel() );
    clusterRecordBuffer->AddRecord( clusterRecord );
  }
  
  return clusterRecordBuffer;
}


vtkRecordBuffer* vtkRecordBuffer
::TrimBufferByLabel( std::vector<std::string> labels )
{
  vtkRecordBuffer* trimRecordBuffer = vtkRecordBuffer::New();

  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    // Check if the current record satisfies one of the labels
    for ( int j = 0; j < labels.size(); j++ )
	{
      if ( labels.at(j).compare( this->GetRecordAt(i)->GetLabel() ) == 0 )
	  {
        trimRecordBuffer->AddRecord( this->GetRecordAt(i)->DeepCopy() );
	  }
	}
  }

  return trimRecordBuffer;
}


std::vector<vtkRecordBuffer*> vtkRecordBuffer
::SplitBufferByLabel( std::vector<std::string> labels )
{
  // Let us assume that all labels will exist
  // The buffers must be in the same order as the labels
  // Otherwise an empty buffer is ok
  std::vector<vtkRecordBuffer*> labelBuffers;
  for ( int i = 0; i < labels.size(); i++ )
  {
    vtkRecordBuffer* currLabelBuffer = vtkRecordBuffer::New();
	labelBuffers.push_back( currLabelBuffer );
  }

  // Separate the transforms by their task name
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    // Ensure that the label for the current record is valid
    int labelIndex = -1;
    for ( int j = 0; j < labels.size(); j++ )
	{
      if ( labels.at(j).compare( this->GetRecordAt(i)->GetLabel() ) == 0 )
	  {
		labelIndex = j;
	  }
	}

	if ( labelIndex < 0 )
	{
      continue;
	}

	labelBuffers.at(labelIndex)->AddRecord( this->GetRecordAt(i)->DeepCopy() );
  }

  return labelBuffers;
}


vtkRecordBuffer* vtkRecordBuffer
::AddCompletion( double time )
{
  // Start from the back - if the label changes then start the completion label until time is expired or the next task
  // Only do it for each instance of task
  vtkRecordBuffer* CompletionBuffer = this->DeepCopy();
  std::vector<std::string> completedLabels;
  std::string finishLabel = "";
  double finishTime = CompletionBuffer->GetCurrentRecord()->GetTime() + time;
  bool completedLabel = false;

  for ( int i = CompletionBuffer->GetNumRecords() - 1; i >= 0; i-- )
  {
    if ( CompletionBuffer->GetRecordAt(i)->GetLabel().compare( finishLabel ) != 0 )
	{
      finishLabel = CompletionBuffer->GetRecordAt(i)->GetLabel();
	  finishTime = CompletionBuffer->GetRecordAt(i)->GetTime();

	  /*
	  bool completedLabel = false;
	  for ( int i = 0; i < completedLabels.size(); i++ )
	  {
        if ( finishLabel.compare( completedLabels.at(i) ) == 0 )
		{
          completedLabel = true;
		}
	  }

	  if ( completedLabel )
	  {
        finishTime += time;
	  }
	  else
	  {
	    completedLabels.push_back( finishLabel );
	  }
      */
      completedLabels.push_back( finishLabel );
	}

    if ( finishTime - CompletionBuffer->GetRecordAt(i)->GetTime() < time )
	{
      CompletionBuffer->GetRecordAt(i)->SetLabel( finishLabel + "_Completion" );
	}
  }

  return CompletionBuffer;
}


std::vector<vtkMarkovRecord*> vtkRecordBuffer
::ToMarkovRecordVector()
{
  std::vector<vtkMarkovRecord*> markovRecords;

  // We will assume that: label -> state, values[0] -> symbol
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    vtkMarkovRecord* currMarkovRecord = vtkMarkovRecord::New();
    currMarkovRecord->SetState( this->GetRecordAt(i)->GetLabel() );
	currMarkovRecord->SetSymbol( this->GetRecordAt(i)->Get(0) );
	markovRecords.push_back( currMarkovRecord );
  }

  return markovRecords;
}