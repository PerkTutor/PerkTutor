
#include "vtkTransformRecord.h"

vtkStandardNewMacro( vtkTransformRecord );


vtkTransformRecord
::vtkTransformRecord()
{
  this->Transform = "";
  this->DeviceName = "";
  this->TimeStampSec = 0;
  this->TimeStampNSec = 0;
}


vtkTransformRecord
::~vtkTransformRecord()
{
  this->Transform = "";
  this->DeviceName = "";
  this->TimeStampSec = 0;
  this->TimeStampNSec = 0;
}



vtkTransformRecord* vtkTransformRecord
::DeepCopy()
{
  vtkTransformRecord* copy = vtkTransformRecord::New();
  copy->SetTransform( this->Transform );
  copy->SetDeviceName( this->DeviceName );
  copy->SetTimeStampSec( this->TimeStampSec );
  copy->SetTimeStampNSec( this->TimeStampNSec );
  return copy;
}



void vtkTransformRecord
::SetTransform( std::string newTransform )
{
  this->Transform = newTransform;
}


std::string vtkTransformRecord
::GetTransform()
{
  return this->Transform;
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


void vtkTransformRecord
::SetTimeStampSec( int newTimeStampSec )
{
  this->TimeStampSec = newTimeStampSec;
}


int vtkTransformRecord
::GetTimeStampSec()
{
  return this->TimeStampSec;
}


void vtkTransformRecord
::SetTimeStampNSec( int newTimeStampNSec )
{
  this->TimeStampNSec = newTimeStampNSec;
}


int vtkTransformRecord
::GetTimeStampNSec()
{
  return this->TimeStampNSec;
}


void vtkTransformRecord
::SetTime( double newTime )
{
  this->TimeStampSec = floor( newTime );
  this->TimeStampNSec = 1.0e+9 * ( newTime - floor( newTime ) ); 
}


double vtkTransformRecord
::GetTime()
{
  return ( this->TimeStampSec + 1.0e-9 * this->TimeStampNSec );
}


std::string vtkTransformRecord
::ToXMLString( int indent )
{
  std::stringstream xmlstring;
  xmlstring << std::string( indent, ' ' );
  xmlstring << "<log";
  xmlstring << " TimeStampSec=\"" << this->TimeStampSec << "\"";
  xmlstring << " TimeStampNSec=\"" << this->TimeStampNSec << "\"";
  xmlstring << " type=\"transform\"";
  xmlstring << " DeviceName=\"" << this->DeviceName << "\"";
  xmlstring << " transform=\"" << this->Transform << "\"";
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

  this->Transform = element->GetAttribute( "transform" );
  this->DeviceName = element->GetAttribute( "DeviceName" );
  this->TimeStampSec = atoi( element->GetAttribute( "TimeStampSec" ) );
  this->TimeStampNSec = atoi( element->GetAttribute( "TimeStampNSec" ) );
}