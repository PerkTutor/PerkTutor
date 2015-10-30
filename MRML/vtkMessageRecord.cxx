
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
::Copy( vtkLogRecord* otherRecord )
{
  this->vtkLogRecord::Copy( otherRecord );

  vtkMessageRecord* messageRecord = vtkMessageRecord::SafeDownCast( otherRecord ); 
  if ( messageRecord == NULL )
  {
    return;
  }

  this->SetMessageString( messageRecord->GetMessageString() );
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
::ToXMLString( vtkIndent indent )
{
  std::stringstream xmlstring;
  xmlstring << indent;
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
  if ( element == NULL || strcmp( element->GetName(), "log" ) != 0 || element->GetAttribute( "type" ) == NULL || strcmp( element->GetAttribute( "type" ), "message" ) != 0 )
  {
    return;
  }

  this->vtkLogRecord::FromXMLElement( element );
  
  if ( element->GetAttribute( "message" ) != NULL )
  {
    this->SetMessageString( std::string( element->GetAttribute( "message" ) ) );
  }
}