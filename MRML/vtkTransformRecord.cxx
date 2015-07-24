
#include "vtkTransformRecord.h"

vtkStandardNewMacro( vtkTransformRecord );


vtkTransformRecord
::vtkTransformRecord()
{
  this->TransformString = "";
  this->DeviceName = "";
}


vtkTransformRecord
::~vtkTransformRecord()
{
  this->TransformString = "";
  this->DeviceName = "";
}


void vtkTransformRecord
::Copy( vtkLogRecord* otherRecord )
{
  this->vtkLogRecord::Copy( otherRecord );

  vtkTransformRecord* transformRecord = vtkTransformRecord::SafeDownCast( otherRecord );
  if ( transformRecord == NULL )
  {
    return;
  }

  this->SetTransformString( transformRecord->GetTransformString() );
  this->SetDeviceName( transformRecord->GetDeviceName() );
}



void vtkTransformRecord
::SetTransformString( std::string newTransformString )
{
  this->TransformString = newTransformString;
}


std::string vtkTransformRecord
::GetTransformString()
{
  return this->TransformString;
}


void vtkTransformRecord
::SetDeviceName( std::string newDeviceName )
{
  this->DeviceName = newDeviceName;
}


std::string vtkTransformRecord
::GetDeviceName()
{
  return this->DeviceName;
}


std::string vtkTransformRecord
::ToXMLString( vtkIndent indent )
{
  std::stringstream xmlstring;
  xmlstring << indent;
  xmlstring << "<log";
  xmlstring << " TimeStampSec=\"" << this->TimeStampSec << "\"";
  xmlstring << " TimeStampNSec=\"" << this->TimeStampNSec << "\"";
  xmlstring << " type=\"transform\"";
  xmlstring << " DeviceName=\"" << this->DeviceName << "\"";
  xmlstring << " transform=\"" << this->TransformString << "\"";
  xmlstring << " />" << std::endl;
  return xmlstring.str();
}


void vtkTransformRecord
::FromXMLElement( vtkXMLDataElement* element )
{
  if ( element == NULL || strcmp( element->GetName(), "log" ) != 0 || strcmp( element->GetAttribute( "type" ), "transform" ) != 0 )
  {
    return;
  }

  this->TransformString = element->GetAttribute( "transform" );
  this->DeviceName = element->GetAttribute( "DeviceName" );
  this->TimeStampSec = atoi( element->GetAttribute( "TimeStampSec" ) );
  this->TimeStampNSec = atoi( element->GetAttribute( "TimeStampNSec" ) );
}