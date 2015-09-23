
#include "vtkTransformRecord.h"

vtkStandardNewMacro( vtkTransformRecord );


vtkTransformRecord
::vtkTransformRecord()
{
  this->TransformMatrix = "";
  this->DeviceName = "";
}


vtkTransformRecord
::~vtkTransformRecord()
{
  this->TransformMatrix = "";
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

  this->SetTransformMatrix( transformRecord->GetTransformMatrix() );
  this->SetDeviceName( transformRecord->GetDeviceName() );
}


void vtkTransformRecord
::SetTransformMatrix( vtkMatrix4x4* newMatrix4x4 )
{
  std::stringstream ss;

  for ( int i = 0; i < 4; i++ )
  {
    for ( int j = 0; j < 4; j++ )
    {
      ss << newMatrix4x4->GetElement( i, j ) << " ";
    }
  }

  this->TransformMatrix = ss.str();
}


void vtkTransformRecord
::SetTransformMatrix( double* newMatrixDouble )
{
  std::stringstream ss;

  for ( int i = 0; i < 16; i++ )
  {
    ss << newMatrixDouble[ i ] << " ";
  }

  this->TransformMatrix = ss.str();
}


void vtkTransformRecord
::SetTransformMatrix( std::string newMatrixString )
{
  this->TransformMatrix = newMatrixString;
}


void vtkTransformRecord
::GetTransformMatrix( vtkMatrix4x4* matrix4x4 )
{
  std::stringstream ss( this->TransformMatrix );

  double value;

  for ( int i = 0; i < 4; i++ )
  {
    for ( int j = 0; j < 4; j++ )
    {
      ss >> value;
      matrix4x4->SetElement( i, j, value );
    }
  }
  
}


void vtkTransformRecord
::GetTransformMatrix( double* matrixDouble )
{
  std::stringstream ss( this->TransformMatrix );

  for ( int i = 0; i < 16; i++ )
  {
    ss >> matrixDouble[ i ];
  }
}


std::string vtkTransformRecord
::GetTransformMatrix()
{
  return this->TransformMatrix;
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
  xmlstring << " transform=\"" << this->TransformMatrix << "\"";
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

  this->TransformMatrix = element->GetAttribute( "transform" );
  this->DeviceName = element->GetAttribute( "DeviceName" );
  this->TimeStampSec = atoi( element->GetAttribute( "TimeStampSec" ) );
  this->TimeStampNSec = atoi( element->GetAttribute( "TimeStampNSec" ) );
}