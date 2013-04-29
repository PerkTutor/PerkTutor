
#include "vtkMessageRecord.h"

vtkStandardNewMacro( vtkMessageRecord );


vtkMessageRecord
::vtkMessageRecord()
{
  this->Message = "";
  this->TimeStampSec = 0;
  this->TimeStampNSec = 0;
}


vtkMessageRecord
::~vtkMessageRecord()
{
}



vtkMessageRecord* vtkMessageRecord
::DeepCopy()
{
  vtkMessageRecord* copy = vtkMessageRecord::New();
  copy->SetMessage( this->Message );
  copy->SetTimeStampSec( this->TimeStampSec );
  copy->SetTimeStampNSec( this->TimeStampNSec );
  return copy;
}



void vtkMessageRecord
::SetMessage( std::string newMessage )
{
  this->Message = newMessage;
}


std::string vtkMessageRecord
::GetMessage()
{
  return this->Message;
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
  this->TimeStampNSec = 1e-9 * ( newTime - floor( newTime ) ); 
}


double vtkMessageRecord
::GetTime()
{
  return ( this->TimeStampSec + 1.0e-9 * TimeStampNSec );
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
  xmlstring << " message=\"" << this->Message << "\"";
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

  this->Message = element->GetAttribute( "message" );
  this->TimeStampSec = atoi( element->GetAttribute( "TimeStampSec" ) );
  this->TimeStampNSec = atoi( element->GetAttribute( "TimeStampNSec" ) );
}