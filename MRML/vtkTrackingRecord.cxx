
#include "vtkTrackingRecord.h"

vtkStandardNewMacro( vtkTrackingRecord );


vtkTrackingRecord
::vtkTrackingRecord()
{
}


vtkTrackingRecord
::~vtkTrackingRecord()
{
}


vtkTrackingRecord* vtkTrackingRecord
::DeepCopy()
{
  vtkTrackingRecord* newTrackingRecord = vtkTrackingRecord::New();
  newTrackingRecord->SetValues( this->GetValues() ); // Observe that this does a deep copy
  newTrackingRecord->SetLabel( this->GetLabel() );
  newTrackingRecord->SetTime( this->GetTime() );
  return newTrackingRecord;
}


void vtkTrackingRecord
::FromString( std::string s )
{
  this->vtkLabelRecord::FromString( s, TRACKINGRECORD_SIZE );
}


std::string vtkTrackingRecord
::ToMatrixString()
{
  if ( this->Size() != TRACKINGRECORD_SIZE )
  {
    return "";
  }

  std::stringstream matrixstring;

  matrixstring << this->Get(3) * this->Get(3) - this->Get(4) * this->Get(4) - this->Get(5) * this->Get(5) + this->Get(6) * this->Get(6) << " ";
  matrixstring << 2 * this->Get(3) * this->Get(4) + 2 * this->Get(5) * this->Get(6) << " ";
  matrixstring << 2 * this->Get(3) * this->Get(5) - 2 * this->Get(4) * this->Get(6) << " ";
  matrixstring << this->Get(0) << " ";

  matrixstring << 2 * this->Get(3) * this->Get(4) - 2 * this->Get(5) * this->Get(6) << " ";
  matrixstring << - this->Get(3) * this->Get(3) + this->Get(4) * this->Get(4) - this->Get(5) * this->Get(5) + this->Get(6) * this->Get(6) << " ";
  matrixstring << 2 * this->Get(4) * this->Get(5) + 2 * this->Get(3) * this->Get(6) << " ";
  matrixstring << this->Get(1) << " ";

  matrixstring << 2 * this->Get(3) * this->Get(5) + 2 * this->Get(4) * this->Get(6) << " ";
  matrixstring << 2 * this->Get(4) * this->Get(5) - 2 * this->Get(3) * this->Get(6) << " ";
  matrixstring << - this->Get(3) * this->Get(3) - this->Get(4) * this->Get(4) + this->Get(5) * this->Get(5) + this->Get(6) * this->Get(6) << " ";
  matrixstring << this->Get(2) << " ";

  matrixstring << 0 << " ";
  matrixstring << 0 << " ";
  matrixstring << 0 << " ";
  matrixstring << 1 << " ";

  return matrixstring.str();
}


void vtkTrackingRecord
::FromMatrixString( std::string newMatrixString )
{

  std::stringstream matrixstring( newMatrixString );
  
  double m11; matrixstring >> m11; double m12; matrixstring >> m12; double m13; matrixstring >> m13; double m14; matrixstring >> m14;
  double m21; matrixstring >> m21; double m22; matrixstring >> m22; double m23; matrixstring >> m23; double m24; matrixstring >> m24;
  double m31; matrixstring >> m31; double m32; matrixstring >> m32; double m33; matrixstring >> m33; double m34; matrixstring >> m34;
  double m41; matrixstring >> m41; double m42; matrixstring >> m42; double m43; matrixstring >> m43; double m44; matrixstring >> m44;

  // Set the translational components
  this->SetValues( std::vector<double>( TRACKINGRECORD_SIZE, 0.0 ) );
  this->Set( 0, m14 );
  this->Set( 1, m24 );
  this->Set( 2, m34 );


  // Use the algorithm from [LandisMarkley 2008]
  std::vector<double> qt;
  qt.push_back( 1 + m11 - m22 - m33 );
  qt.push_back( 1 - m11 + m22 - m33 );
  qt.push_back( 1 - m11 - m22 + m33 );
  qt.push_back( 1 + m11 + m22 + m33 );

  // Find the largest element
  int maxIndex = 0;

  for ( int i = 0; i < 4; i++ )
  {
	if ( qt.at(i) > qt.at(maxIndex) )
	{
	  maxIndex = i;
	}
  }

  // Choose the appropriate quaternion calculation
  if (maxIndex == 0)
  {
    this->Set( 3, 1 + m11 - m22 - m33 );
    this->Set( 4, m12 + m21 );
	this->Set( 5, m13 + m31 );
	this->Set( 6, m23 - m32 );
  }

  if (maxIndex == 1)
  {
    this->Set( 3, m21 + m12 );
	this->Set( 4, 1 - m11 + m22 - m33 );
	this->Set( 5, m23 + m32 );
	this->Set( 6, m31 - m13 );
  }

  if (maxIndex == 2)
  {
    this->Set( 3, m31 + m13 );
	this->Set( 4, m32 + m23 );
	this->Set( 5, 1 - m11 - m22 + m33 );
	this->Set( 6, m12 - m21 );
  }

  if (maxIndex == 3)
  {
    this->Set( 3, m23 - m32 );
	this->Set( 4, m31 - m13 );
	this->Set( 5, m12 - m21 );
	this->Set( 6, 1 + m11 + m22 + m33 );
  }

  // Normalize, and ensure q4 is positive
  if ( this->Get(6) < 0 )
  {
    this->Set( 3, - this->Get(3) );
	this->Set( 4, - this->Get(4) );
	this->Set( 5, - this->Get(5) );
	this->Set( 6, - this->Get(6) );
  }

  double norm = sqrt( this->Get(3) * this->Get(3) + this->Get(4) * this->Get(4) + this->Get(5) * this->Get(5) + this->Get(6) * this->Get(6) );

  this->Set( 3, this->Get(3) / norm );
  this->Set( 4, this->Get(4) / norm );
  this->Set( 5, this->Get(5) / norm );
  this->Set( 6, this->Get(6) / norm );

}


vtkTransformRecord* vtkTrackingRecord
::ToTransformRecord()
{
  vtkTransformRecord* transformRecord = vtkTransformRecord::New();
  transformRecord->SetTime( this->GetTime() );
  transformRecord->SetDeviceName( this->GetLabel() );
  transformRecord->SetTransform( this->ToMatrixString() );
  return transformRecord;
}


void vtkTrackingRecord
::FromTransformRecord( vtkTransformRecord* newTransformRecord )
{
  this->SetTime( newTransformRecord->GetTime() );
  this->SetLabel( newTransformRecord->GetDeviceName() );
  this->FromMatrixString( newTransformRecord->GetTransform() );
}


std::string vtkTrackingRecord
::ToXMLString( std::string name )
{
  std::stringstream xmlstring;

  xmlstring << "  <log";
  xmlstring << " TimeStampSec=\"" << this->GetSec() << "\"";
  xmlstring << " TimeStampNSec=\"" << this->GetNSec() << "\"";
  xmlstring << " type=\"transform\"";
  xmlstring << " DeviceName=\"" << name << "\"";
  xmlstring << " transform=\"" << this->ToMatrixString() << "\"";
  xmlstring << " />" << std::endl;

  return xmlstring.str();
}


void vtkTrackingRecord
::FromXMLElement( vtkXMLDataElement* element, std::string name )
{

  if ( strcmp( element->GetName(), "log" ) != 0 || strcmp( element->GetAttribute( "type" ), "transform" ) != 0 )
  {
    return;  // If it's not a "log" or is the wrong tool jump to the next.
  }
  if ( strcmp( element->GetAttribute( "DeviceName" ), name.c_str() ) != 0 && strcmp( "", name.c_str() ) != 0 )
  {
    return;
  }

  this->FromMatrixString( std::string( element->GetAttribute( "transform" ) ) );
  this->SetTime( atoi( element->GetAttribute( "TimeStampSec" ) ), atoi( element->GetAttribute( "TimeStampNSec" ) ) );
}