
#include "vtkLogRecord.h"

vtkStandardNewMacro( vtkLogRecord );


vtkLogRecord
::vtkLogRecord()
{
  this->TimeStampSec = 0;
  this->TimeStampNSec = 0;
}


vtkLogRecord
::~vtkLogRecord()
{
  this->TimeStampSec = 0;
  this->TimeStampNSec = 0;
}


void vtkLogRecord
::Copy( vtkLogRecord* otherRecord )
{
  if ( otherRecord == NULL )
  {
    return;
  }

  this->SetTimeStampSec( otherRecord->GetTimeStampSec() );
  this->SetTimeStampNSec( otherRecord->GetTimeStampNSec() );
}


void vtkLogRecord
::SetTimeStampSec( int newTimeStampSec )
{
  this->TimeStampSec = newTimeStampSec;
}


int vtkLogRecord
::GetTimeStampSec()
{
  return this->TimeStampSec;
}


void vtkLogRecord
::SetTimeStampNSec( int newTimeStampNSec )
{
  this->TimeStampNSec = newTimeStampNSec;
}


int vtkLogRecord
::GetTimeStampNSec()
{
  return this->TimeStampNSec;
}


void vtkLogRecord
::SetTime( double newTime )
{
  this->TimeStampSec = floor( newTime );
  this->TimeStampNSec = 1.0e+9 * ( newTime - floor( newTime ) ); 
}


double vtkLogRecord
::GetTime()
{
  return ( this->TimeStampSec + 1.0e-9 * this->TimeStampNSec );
}


std::string vtkLogRecord
::ToXMLString( vtkIndent indent )
{
  std::stringstream xmlstring;
  xmlstring << indent;
  xmlstring << "<log";
  xmlstring << " TimeStampSec=\"" << this->TimeStampSec << "\"";
  xmlstring << " TimeStampNSec=\"" << this->TimeStampNSec << "\"";
  xmlstring << " />" << std::endl;
  return xmlstring.str();
}


void vtkLogRecord
::FromXMLElement( vtkXMLDataElement* element )
{
  if ( element == NULL || strcmp( element->GetName(), "log" ) != 0 )
  {
    return;
  }
  
  if ( element->GetAttribute( "TimeStampSec" ) != NULL )
  {
    this->TimeStampSec = atoi( element->GetAttribute( "TimeStampSec" ) );
  }
  if ( element->GetAttribute( "TimeStampNSec" ) != NULL )
  {
    this->TimeStampNSec = atoi( element->GetAttribute( "TimeStampNSec" ) );
  }
}