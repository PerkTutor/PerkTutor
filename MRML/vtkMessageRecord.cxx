
#include "vtkMessageRecord.h"

vtkStandardNewMacro( vtkMessageRecord );


vtkMessageRecord
::vtkMessageRecord()
{
  this->Name = "";
  this->TimeStampSec = 0;
  this->TimeStampNSec = 0;
}


vtkMessageRecord
::~vtkMessageRecord()
{
  this->Name = "";
  this->TimeStampSec = 0;
  this->TimeStampNSec = 0;
}



vtkMessageRecord* vtkMessageRecord
::DeepCopy()
{
  vtkMessageRecord* copy = vtkMessageRecord::New();
  copy->SetName( this->Name );
  copy->SetTimeStampSec( this->TimeStampSec );
  copy->SetTimeStampNSec( this->TimeStampNSec );
  return copy;
}



void vtkMessageRecord
::SetName( std::string newName )
{
  this->Name = newName;
}


std::string vtkMessageRecord
::GetName()
{
  return this->Name;
}


void vtkMessageRecord
::SetTimeStampSec( int newTimeStampSec )
{
  this->TimeStampSec = newTimeStampSec;
}


int vtkMessageRecord
::GetTimeStampSec()
{
  return this->TimeStampSec;
}


void vtkMessageRecord
::SetTimeStampNSec( int newTimeStampNSec )
{
  this->TimeStampNSec = newTimeStampNSec;
}


int vtkMessageRecord
::GetTimeStampNSec()
{
  return this->TimeStampNSec;
}


void vtkMessageRecord
::SetTime( double newTime )
{
  this->TimeStampSec = floor( newTime );
  this->TimeStampNSec = 1.0e+9 * ( newTime - floor( newTime ) ); 
}


double vtkMessageRecord
::GetTime()
{
  return ( this->TimeStampSec + 1.0e-9 * this->TimeStampNSec );
}


std::string vtkMessageRecord
::ToXMLString( int indent )
{
  std::stringstream xmlstring;
  xmlstring << std::string( indent, ' ' );
  xmlstring << "<log";
  xmlstring << " TimeStampSec=\"" << this->TimeStampSec << "\"";
  xmlstring << " TimeStampNSec=\"" << this->TimeStampNSec << "\"";
  xmlstring << " type=\"message\"";
  xmlstring << " message=\"" << this->Name << "\"";
  xmlstring << " />" << std::endl;
  return xmlstring.str();
}


void vtkMessageRecord
::FromXMLElement( vtkXMLDataElement* element )
{
  if ( element == NULL || strcmp( element->GetName(), "log" ) != 0 || strcmp( element->GetAttribute( "type" ), "message" ) != 0 )
  {
    return;
  }

  this->Name = element->GetAttribute( "message" );
  this->TimeStampSec = atoi( element->GetAttribute( "TimeStampSec" ) );
  this->TimeStampNSec = atoi( element->GetAttribute( "TimeStampNSec" ) );
}