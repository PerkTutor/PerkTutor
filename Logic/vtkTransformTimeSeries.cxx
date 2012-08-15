
#include "vtkTransformTimeSeries.h"


#include <limits>

#include "vtkObjectFactory.h"



vtkStandardNewMacro( vtkTransformTimeSeries );



void
vtkTransformTimeSeries
::PrintSelf( ostream& os, vtkIndent indent )
{
  Superclass::PrintSelf( os, indent );
}



void
vtkTransformTimeSeries
::Copy( vtkTransformTimeSeries* from )
{
  if ( from == NULL )
  {
    return;
  }
  
  for ( unsigned int i = 0; i < from->GetNumberOfRecords(); ++ i )
  {
    this->Data.push_back( from->Data[ i ] );
  }
  this->MinTime = from->MinTime;
  this->MaxTime = from->MaxTime;
}



int
vtkTransformTimeSeries
::GetNumberOfRecords() const
{
  return this->Data.size();
}



double
vtkTransformTimeSeries
::GetTimeAtIndex( int index ) const
{
  if ( index < 0  ||  (unsigned int)index >= this->Data.size() )
    {
      // TODO: Exception.
    return -1.0;
    }
  
  return this->Data[ index ].first;
}



vtkTransform*
vtkTransformTimeSeries
::GetTransformAtIndex( int index ) const
{
  if ( index < 0  ||  (unsigned int)index >= this->Data.size() )
    {
      // TODO: Exception.
    return 0;
    }
  
  return this->Data[ index ].second;
}



void
vtkTransformTimeSeries
::AddRecord( double time, vtkTransform* transform )
{
  vtkTransform* tr = vtkTransform::New(); // Deleted in Clear function.
  tr->DeepCopy( transform );
  
  std::pair< double, vtkTransform* > p( time, tr );
  
  
  if ( time < this->MinTime )
    {
    this->Data.insert( this->Data.begin(), p );
    this->MinTime = time;
    }
  
  else if ( time >= this->MaxTime )
    {
    this->Data.push_back( p );
    this->MaxTime = time;
    }
    
  else
    {
    for ( DataIteratorType it = this->Data.begin(); it < this->Data.end(); ++ it )
      {
      if ( (*it).first > time )
        {
        DataIteratorType insertionPoint = it;  // Pushes the item at "it" to a higher index, and replaces it.
        this->Data.insert( insertionPoint, p );
        break;
        }
      }
    }
}



void
vtkTransformTimeSeries
::Clear()
{
  for ( DataIteratorType it = this->Data.begin(); it < this->Data.end(); ++ it )
    {
    (*it).second->Delete();
    (*it).second = NULL;
    }
  
  this->Data.clear();
  
  this->MinTime = std::numeric_limits< double >::max();
  this->MaxTime = std::numeric_limits< double >::min();
}



vtkTransformTimeSeries
::vtkTransformTimeSeries()
{
  this->MinTime = std::numeric_limits< double >::max();
  this->MaxTime = std::numeric_limits< double >::min();
}



vtkTransformTimeSeries
::~vtkTransformTimeSeries()
{
  this->Clear();
}

