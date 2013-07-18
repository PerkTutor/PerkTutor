
#include "vtkLabelRecord.h"

vtkStandardNewMacro( vtkLabelRecord );


vtkLabelRecord
::vtkLabelRecord()
{
  this->Time = 0;
}


vtkLabelRecord
::~vtkLabelRecord()
{
}


vtkLabelRecord* vtkLabelRecord
::DeepCopy()
{
  vtkLabelRecord* newLabelRecord = vtkLabelRecord::New();
  newLabelRecord->SetValues( this->GetValues() ); // Observe that this does a deep copy
  newLabelRecord->SetLabel( this->GetLabel() );
  newLabelRecord->SetTime( this->GetTime() );
  return newLabelRecord;
}


double vtkLabelRecord
::GetTime()
{
  return this->Time;
}


int vtkLabelRecord
::GetSec()
{
  return floor( this->Time );
}


int vtkLabelRecord
::GetNSec()
{
  return floor( 1.0e9 * ( this->Time - floor( this->Time ) ) );
}


void vtkLabelRecord
::SetTime( double newTime )
{
  this->Time = newTime;
}


void vtkLabelRecord
::SetTime( int newSec, int newNSec )
{
  this->Time = newSec + 1.0e-9 * newNSec;
}


std::string vtkLabelRecord
::ToXMLString( std::string name )
{
  std::stringstream xmlstring;

  xmlstring << "      <" << name;
  xmlstring << " TimeStampSec=\"" << this->GetSec() << "\"";
  xmlstring << " TimeStampNSec=\"" << this->GetNSec() << "\"";
  xmlstring << " Label=\"" << this->GetLabel() << "\"";
  xmlstring << " Size=\"" << this->Size() << "\"";
  xmlstring << " Values=\"" << this->ToString() << "\"";
  xmlstring << " />" << std::endl;

  return xmlstring.str();
}


void vtkLabelRecord
::FromXMLElement( vtkXMLDataElement* element, std::string name )
{

  if ( strcmp( element->GetName(), name.c_str() ) != 0 )
  {
    return;  // If it's not a "log" or is the wrong tool jump to the next.
  }

  this->FromString( std::string( element->GetAttribute( "Values" ) ), atoi( element->GetAttribute( "Size" ) ) );
  this->SetTime( atoi( element->GetAttribute( "TimeStampSec" ) ), atoi( element->GetAttribute( "TimeStampNSec" ) ) );
  this->SetLabel( std::string( element->GetAttribute( "Label" ) ) );
}