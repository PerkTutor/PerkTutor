
#include "vtkLabelRecord.h"

vtkStandardNewMacro( vtkLabelRecord );


vtkLabelRecord
::vtkLabelRecord()
{
  this->Vector = vtkSmartPointer< vtkLabelVector >::New();
}


vtkLabelRecord
::~vtkLabelRecord()
{
}


void vtkLabelRecord
::Copy( vtkLabelRecord* otherRecord )
{
  this->vtkLogRecord::Copy( otherRecord );
  
  this->GetVector()->Copy( otherRecord->GetVector() );
}


void vtkTrackingRecord
::ToTransformRecord( vtkTransformRecord* transformRecord, TrackingRecordType type )
{
  std::stringstream matrixString;
  if ( type == QUATERNION_RECORD && this->GetVector()->Size() == QUATERNION_RECORD ) // If it is in quaternion format
  {
    double quaternion[ 4 ];
    quaternion[ 0 ] = this->GetVector()->GetElement( 3 );
    quaternion[ 1 ] = this->GetVector()->GetElement( 4 );
    quaternion[ 2 ] = this->GetVector()->GetElement( 5 );
    quaternion[ 3 ] = this->GetVector()->GetElement( 6 );
    
    double matrix[ 3 ][ 3 ];
    vtkMath::QuaternionToMatrix3x3( quaternion, matrix );
    
    matrixString << matrix[ 0 ][ 0 ] << matrix[ 0 ][ 1 ] << matrix[ 0 ][ 2 ] << this->GetVector()->GetElement( 0 );
    matrixString << matrix[ 1 ][ 0 ] << matrix[ 1 ][ 1 ] << matrix[ 1 ][ 2 ] << this->GetVector()->GetElement( 1 );
    matrixString << matrix[ 2 ][ 0 ] << matrix[ 2 ][ 1 ] << matrix[ 2 ][ 2 ] << this->GetVector()->GetElement( 2 );
    matrixString << 0 << 0 << 0 << 1;
  }
  else if ( type == MATRIX_RECORD && this->GetVector()->Size() == MATRIX_RECORD ) // If it is in matrix format
  {
    matrixString << this->GetVector()->ToString();
  }
  else
  {
    return;
  }
  
  transformRecord->SetTime( this->GetTime() );
  transformRecord->SetDeviceName( this->GetLabel() );
  transformRecord->SetTransformString( matrixString.str() );
}


void vtkTrackingRecord
::FromTransformRecord( vtkTransformRecord* transformRecord, TrackingRecordType type )
{
  std::stringstream matrixString( transformRecord->GetTransformString() );
  std::stringstream trackingString;
  if ( type == QUATERNION_RECORD ) // If it is in quaternion format
  {
    double translation[ 3 ][ 3 ];
    double matrix[ 3 ][ 3 ];
    matrixString >> matrix[ 0 ][ 0 ]; matrixString >> matrix[ 0 ][ 1 ]; matrixString >> matrix[ 0 ][ 2 ]; matrixString >> translation[ 0 ];
    matrixString >> matrix[ 1 ][ 0 ]; matrixString >> matrix[ 1 ][ 1 ]; matrixString >> matrix[ 1 ][ 2 ]; matrixString >> translation[ 1 ];
    matrixString >> matrix[ 2 ][ 0 ]; matrixString >> matrix[ 2 ][ 1 ]; matrixString >> matrix[ 2 ][ 2 ]; matrixString >> translation[ 2 ];
    
    double quaternion[ 4 ];
    vtkMath::Matrix3x3ToQuaternion( quaternion, matrix );
    
    trackingString << translation[ 0 ] << translation[ 1 ] << translation[ 2 ];
    trackingString << quaternion[ 0 ] << quaternion[ 1 ] << quaternion[ 2 ] << quaternion[ 3 ];
  }
  else if ( type == MATRIX_RECORD ) // If it is in matrix format
  {
    trackingString << transformRecord->GetTransformString();
  }
  else
  {
    return;
  }
  
  this->SetTime( transformRecord->GetTime() );
  this->GetVector()->SetLabel( transformRecord->GetDeviceName() );
  this->GetVector()->FromString( trackingString, type );
}


std::string vtkLabelRecord
::ToXMLString( vtkIndent indent )
{
  std::stringstream xmlstring;

  xmlstring << indent << "<Record";
  xmlstring << " TimeStampSec=\"" << this->GetSec() << "\"";
  xmlstring << " TimeStampNSec=\"" << this->GetNSec() << "\"";
  xmlstring << " Label=\"" << this->GetVector()->GetLabel() << "\"";
  xmlstring << " Size=\"" << this->GetVector()->Size() << "\"";
  xmlstring << " Values=\"" << this->GetVector()->ToString() << "\"";
  xmlstring << " />" << std::endl;

  return xmlstring.str();
}


void vtkLabelRecord
::FromXMLElement( vtkXMLDataElement* element, std::string name )
{
  if ( element == NULL || strcmp( element->GetName(), "Record" ) != 0 )
  {
    return;  // If it's not a "record" jump to the next.
  }

  this->TimeStampSec = atoi( element->GetAttribute( "TimeStampSec" ) );
  this->TimeStampNSec = atoi( element->GetAttribute( "TimeStampNSec" ) );
  this->FromString( std::string( element->GetAttribute( "Values" ) ), atoi( element->GetAttribute( "Size" ) ) );
  this->SetLabel( std::string( element->GetAttribute( "Label" ) ) );
}