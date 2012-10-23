
#include "vtkTrackingRecord.h"
#include "vtkProcedure.h"
#include "vtkSmartPointer.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"

#include <string>
#include <sstream>


vtkStandardNewMacro( vtkProcedure );

vtkProcedure
::vtkProcedure()
{
  this->numRecords = 0;
}


vtkProcedure
::~vtkProcedure()
{
  // Iterate over all items in the vector and delete
  for( int i = 0; i < numRecords; i++ )
    delete [] records.at(i);

  records.clear();
}

vtkProcedure* vtkProcedure
::DeepCopy()
{
  // Create a new procedure object
  vtkProcedure* newProc = vtkProcedure::New();

  // Iterate over all tracking records in the current procedure, and add to new
  for ( int i = 0; i < numRecords; i++ )
  {
    newProc->AddTrackingRecord( records.at(i)->DeepCopy() );
  }

  return newProc;
}


void vtkProcedure
::AddTrackingRecord( vtkTrackingRecord* tr )
{
  records.push_back( tr );
  numRecords++;
}

int vtkProcedure
::Size()
{
  return numRecords;
}