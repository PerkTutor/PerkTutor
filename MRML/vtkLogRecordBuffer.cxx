

#include "vtkLogRecordBuffer.h"

vtkStandardNewMacro( vtkLogRecordBuffer );


// Constructors and Destructors --------------------------------------------------------------------

vtkLogRecordBuffer
::vtkLogRecordBuffer()
{
  // No need to initialize the vectors
}


vtkLogRecordBuffer
::~vtkLogRecordBuffer()
{
  this->Clear();
}



// Records ------------------------------------------------------------------------

void vtkLogRecordBuffer
::Copy( vtkLogRecordBuffer* otherBuffer )
{
  if ( otherBuffer == NULL )
  {
    return;
  }

  // Copy all of the records
  for ( int i = 0; i < otherBuffer->GetNumRecords(); i++ )
  {
    vtkSmartPointer< vtkLogRecord > newRecord = vtkSmartPointer< vtkLogRecord >::New();
    newRecord->Copy( otherBuffer->GetRecord( i ) );
    this->AddRecord( newRecord );
  }

}


void vtkLogRecordBuffer
::Concatenate( vtkLogRecordBuffer* catBuffer )
{
  // If you want things to be deep copied, just deep copy the buffer
  for ( int i = 0; i < catBuffer->GetNumRecords(); i++ )
  {
    this->AddRecord( catBuffer->GetRecord( i ) );
  }
}


int vtkLogRecordBuffer
::AddRecord( vtkLogRecord* newRecord )
{
  // Ensure that we put it into sorted order (usually it will go at the end)
  int insertLocation = 0;
  if ( this->GetNumRecords() < 1 )
  {
    this->Records.push_back( newRecord );
	  return 0;
  }
  if ( newRecord->GetTime() >= this->GetCurrentRecord()->GetTime() )
  {
    this->Records.push_back( newRecord );
	  return this->GetNumRecords() - 1;
  }
  if ( newRecord->GetTime() <= this->GetRecord(0)->GetTime() )
  {
    this->Records.insert( this->Records.begin() + 0, newRecord );
	  return 0;
  }

  // Use the binary search
  insertLocation = this->GetPriorRecordIndex( newRecord->GetTime() ) + 1;
  this->Records.insert( this->Records.begin() + insertLocation, newRecord );
  return insertLocation;
}


bool vtkLogRecordBuffer
::RemoveRecord( int index )
{
  if ( index >= 0 && index < this->GetNumRecords() )
  {
    this->Records.erase( this->Records.begin() + index );
    return true;
  }
  return false;
}


vtkLogRecord* vtkLogRecordBuffer
::GetRecord( int index )
{
  if ( index >= 0 && index < this->GetNumRecords() )
  {
    return this->Records.at( index );
  }
  return NULL;
}


vtkLogRecord* vtkLogRecordBuffer
::GetCurrentRecord()
{
  return this->Records.at( this->GetNumRecords() - 1 );
}


vtkLogRecord* vtkLogRecordBuffer
::GetRecordAtTime( double time )
{
  // Check boundary times first
  if ( this->GetNumRecords() == 0 )
  {
    return NULL;
  }
  if ( time <= this->GetRecord(0)->GetTime() )
  {
    return this->GetRecord( 0 );
  }
  if ( time >= this->GetCurrentRecord()->GetTime() )
  {
    return this->GetCurrentRecord();
  }

  // Binary search since the records are sorted
  return this->GetRecord( this->GetClosestRecordIndex( time ) );

}


int vtkLogRecordBuffer
::GetNumRecords()
{
  return this->Records.size();
}


// Report the maximum time
// Note: These are absolute times, not relative times
double vtkLogRecordBuffer
::GetMaximumTime()
{
  if ( this->GetNumRecords() > 0 )
  {
    return this->GetCurrentRecord()->GetTime();
  }

  return 0.0;
}


// Report the minimum time
// Note: These are absolute times, not relative times
double vtkLogRecordBuffer
::GetMinimumTime()
{
  if ( this->GetNumRecords() > 0 )
  {
    return this->GetRecord(0)->GetTime();
  }

  return 0.0;
}


double vtkLogRecordBuffer
::GetTotalTime()
{
  return this->GetMaximumTime() - this->GetMinimumTime();
}


void vtkLogRecordBuffer
::Clear()
{
  // No need to explicitly call the VTK delete function, since we use smart pointers for each record
  this->Records.clear();
}


// Implement binary searches to find transforms and/or messages at a particular time
int vtkLogRecordBuffer
::GetPriorRecordIndex( double time )
{
  int lowerBound = 0;
  int upperBound = this->GetNumRecords() - 1;

  if ( time < this->GetRecord( lowerBound )->GetTime() )
  {
    return lowerBound;
  }
  if ( time > this->GetRecord( upperBound )->GetTime() )
  {
    return upperBound;
  }

  while ( upperBound - lowerBound > 1 )
  {
    // Note that if middle is equal to either lowerBound or upperBound then upperBound - lowerBound <= 1
    int middle = int( ( lowerBound + upperBound ) / 2 );
    double middleTime = this->GetRecord( middle )->GetTime();

    if ( time == middleTime )
    {
      return middle;
    }

    if ( time > middleTime )
    {
      lowerBound = middle;
    }

    if ( time < middleTime )
    {
      upperBound = middle;
    }

  }

  // Since we're returning the prior index, always return the lower bound
  return lowerBound;
}


int vtkLogRecordBuffer
::GetClosestRecordIndex( double time )
{
  int lowerIndex = this->GetPriorRecordIndex( time );
  int upperIndex = lowerIndex + 1;

  if ( upperIndex >= this->GetNumRecords() || std::abs ( time - this->GetRecord( lowerIndex )->GetTime() ) < std::abs ( time - this->GetRecord( upperIndex )->GetTime() ) )
  {
    return lowerIndex;
  }
  else
  {
    return upperIndex;
  }
}


std::string vtkLogRecordBuffer
::ToXMLString()
{
  std::stringstream xmlstring;
  
  for ( int i = 0; i < this->GetNumRecords(); i++ )
  {
    xmlstring << this->GetRecord(i)->ToXMLString();
  }

  return xmlstring.str();
}


void vtkLogRecordBuffer
::FromXMLElement( vtkXMLDataElement* rootElement )
{
  if ( ! rootElement || strcmp( rootElement->GetName(), "TransformRecorderLog" ) != 0 ) 
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

    // Note that this is independent of the type of record
    // This will generally be insufficient
	  vtkSmartPointer< vtkLogRecord > newRecord = vtkSmartPointer< vtkLogRecord >::New();
		newRecord->FromXMLElement( element );
		this->AddRecord( newRecord );
   
  }

  // Status updates are taken care of in the add functions
}
