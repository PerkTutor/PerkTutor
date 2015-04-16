
#include "vtkMessageRecord.h"

vtkStandardNewMacro( vtkMessageRecord );


vtkMessageRecord
::vtkMessageRecord()
{
  this->MessageString = "";
}


vtkMessageRecord
::~vtkMessageRecord()
{
  this->MessageString = "";
}



void vtkMessageRecord
::Copy( vtkMessageRecord* otherRecord )
{
  if ( otherRecord == NULL )
  {
    return;
  }

  this->vtkLogRecord::Copy( otherRecord );
  this->SetMessageString( otherRecord->GetMessageString() );
}


void vtkMessageRecord
::SetMessageString( std::string newMessageString )
{
  this->MessageString = newMessageString;
}


std::string vtkMessageRecord
::GetMessageString()
{
  return this->MessageString;
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
  xmlstring << " message=\"" << this->MessageString << "\"";
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

  this->MessageString = element->GetAttribute( "message" );
  this->TimeStampSec = atoi( element->GetAttribute( "TimeStampSec" ) );
  this->TimeStampNSec = atoi( element->GetAttribute( "TimeStampNSec" ) );
}