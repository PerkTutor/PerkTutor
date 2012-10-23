
#include "vtkTrackingRecord.h"
#include "vtkSmartPointer.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"

#include <string>
#include <sstream>


vtkStandardNewMacro( vtkTrackingRecord );


vtkTrackingRecord
::vtkTrackingRecord()
{
  this->time = 0;
  this->deviceName = "";

  this->trans.x = 0;
  this->trans.y = 0;
  this->trans.z = 0;

  this->quat.q1 = 0;
  this->quat.q2 = 0;
  this->quat.q3 = 0;
  this->quat.q4 = 1;
}


vtkTrackingRecord
::~vtkTrackingRecord()
{
  delete [] &trans;
  delete [] &quat;
}

vtkTrackingRecord* vtkTrackingRecord
::DeepCopy()
{
  // Create new translation and rotation objects
  TranslationXYZ newTrans;
  newTrans.x = trans.x;
  newTrans.y = trans.y;
  newTrans.z = trans.z;

  RotationQuaternion newQuat;
  newQuat.q1 = quat.q1;
  newQuat.q2 = quat.q2;
  newQuat.q3 = quat.q3;
  newQuat.q4 = quat.q4;

  vtkTrackingRecord* newRec = vtkTrackingRecord::New();
  newRec->SetTranslation( newTrans );
  newRec->SetRotation( newQuat );

  return newRec;
}


TranslationXYZ vtkTrackingRecord
::GetTranslation()
{
  return trans;
}

void vtkTrackingRecord
::SetTranslation( TranslationXYZ tr )
{
  trans = tr;
}

RotationQuaternion vtkTrackingRecord
::GetRotation()
{
  return quat;
}

void vtkTrackingRecord
::SetRotation( RotationQuaternion q )
{
  quat = q;
}

// Stick together the translation and rotation into a vector
std::vector<double> vtkTrackingRecord
::GetVector()
{
  std::vector<double> v;
  v.push_back( trans.x );
  v.push_back( trans.y );
  v.push_back( trans.z );
  v.push_back( quat.q1 );
  v.push_back( quat.q2 );
  v.push_back( quat.q3 );
  v.push_back( quat.q4 );

  return v;
}

// Unwind the given vector into a translation and rotation
void vtkTrackingRecord
::SetVector( std::vector<double> v )
{
  // Check v is of correct form
  if ( v.size() != 7 )
  {
    return;
  }

  // Stick together the translation and rotation
  trans.x = v.at(0);
  trans.y = v.at(1);
  trans.z = v.at(2);
  quat.q1 = v.at(3);
  quat.q2 = v.at(4);
  quat.q3 = v.at(5);
  quat.q4 = v.at(6);

}


//Return a matrix corresponding to the record
vtkSmartPointer< vtkMatrix4x4 > vtkTrackingRecord
::GetMatrix()
{

  // Create a new 4x4 matrix
  vtkSmartPointer< vtkMatrix4x4 > m = vtkSmartPointer< vtkMatrix4x4 >::New();

  // Set the translational components
  m->SetElement( 0, 3, trans.x );
  m->SetElement( 1, 3, trans.y );
  m->SetElement( 2, 3, trans.z );
  m->SetElement( 3, 3, 1 );

  // Calculate the rotational components
  m->SetElement( 0, 0, quat.q1 * quat.q1 - quat.q2 * quat.q2 - quat.q3 * quat.q3 + quat.q4 * quat.q4 );
  m->SetElement( 0, 1, 2 * quat.q1 * quat.q2 + 2 * quat.q3 * quat.q4 );
  m->SetElement( 0, 2, 2 * quat.q1 * quat.q3 - 2 * quat.q2 * quat.q4 );

  m->SetElement( 1, 0, 2 * quat.q1 * quat.q2 - 2 * quat.q3 * quat.q4 );
  m->SetElement( 1, 1, - quat.q1 * quat.q1 + quat.q2 * quat.q2 - quat.q3 * quat.q3 + quat.q4 * quat.q4 );
  m->SetElement( 1, 2, 2 * quat.q2 * quat.q3 + 2 * quat.q1 * quat.q4 );

  m->SetElement( 2, 0, 2 * quat.q1 * quat.q3 + 2 * quat.q2 * quat.q4 );
  m->SetElement( 2, 1, 2 * quat.q2 * quat.q3 - 2 * quat.q1 * quat.q4 );
  m->SetElement( 2, 2, - quat.q1 * quat.q1 - quat.q2 * quat.q2 + quat.q3 * quat.q3 + quat.q4 * quat.q4 );

  // Return this matrix
  return m;

}



// Set the record using a matrix
void vtkTrackingRecord
::SetMatrix( vtkSmartPointer< vtkMatrix4x4 > m )
{

  // Set the translational components
  trans.x = m->GetElement( 0, 3);
  trans.y = m->GetElement( 1, 3);
  trans.z = m->GetElement( 2, 3);

  // Use the algorithm from [LandisMarkley 2008]
  std::vector<double> qt;

  qt.push_back( 1 + m->GetElement(0,0) - m->GetElement(1,1) - m->GetElement(2,2) );
  qt.push_back( 1 - m->GetElement(0,0) + m->GetElement(1,1) - m->GetElement(2,2) );
  qt.push_back( 1 - m->GetElement(0,0) - m->GetElement(1,1) + m->GetElement(2,2) );
  qt.push_back( 1 + m->GetElement(0,0) + m->GetElement(1,1) + m->GetElement(2,2) );

  // Find the largest element
  int maxIndex = 1;

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
    quat.q1 = 1 + m->GetElement(0,0) - m->GetElement(1,1) - m->GetElement(2,2);
    quat.q2 = m->GetElement(0,1) + m->GetElement(1,0);
    quat.q3 = m->GetElement(0,2) + m->GetElement(2,0);
    quat.q4 = m->GetElement(1,2) - m->GetElement(2,1);
  }

  if (maxIndex == 1)
  {
    quat.q1 = m->GetElement(1,0) + m->GetElement(0,1);
    quat.q2 = 1 - m->GetElement(0,0) + m->GetElement(1,1) - m->GetElement(2,2);
    quat.q3 = m->GetElement(1,2) + m->GetElement(2,1);
    quat.q4 = m->GetElement(2,0) - m->GetElement(0,2);
  }

  if (maxIndex == 2)
  {
    quat.q1 = m->GetElement(2,0) + m->GetElement(0,2);
    quat.q2 = m->GetElement(2,1) + m->GetElement(1,2);
    quat.q3 = 1 - m->GetElement(0,0) - m->GetElement(1,1) + m->GetElement(2,2);
    quat.q4 = m->GetElement(0,1) - m->GetElement(1,0);
  }

  if (maxIndex == 3)
  {
    quat.q1 = m->GetElement(1,2) - m->GetElement(2,1);
    quat.q2 = m->GetElement(2,0) - m->GetElement(0,2);
    quat.q3 = m->GetElement(0,1) - m->GetElement(1,0);
    quat.q4 = 1 + m->GetElement(0,0) + m->GetElement(1,1) + m->GetElement(2,2);
  }

  // Normalize, and ensure q4 is positive
  if ( quat.q4 < 0 )
  {
    quat.q1 = - quat.q1;
	quat.q2 = - quat.q2;
	quat.q3 = - quat.q3;
	quat.q4 = - quat.q4;
  }

  double norm = quat.q1 * quat.q1 + quat.q2 * quat.q2 + quat.q3 * quat.q3 + quat.q4 * quat.q4;

  quat.q1 = quat.q1 / norm;
  quat.q2 = quat.q1 / norm;
  quat.q3 = quat.q1 / norm;
  quat.q4 = quat.q1 / norm;

}

double vtkTrackingRecord
::GetTime()
{
  return time;
}

void vtkTrackingRecord
::SetTime( double t )
{
  time = t;
}

std::string vtkTrackingRecord
::GetDeviceName()
{
  return deviceName;
}

void vtkTrackingRecord
::SetDeviceName( std::string s )
{
  deviceName = s;
}

std::string vtkTrackingRecord
::toString()
{
  std::stringstream ss;
  ss << trans.x << " ";
  ss << trans.y << " ";
  ss << trans.z << " ";
  ss << quat.q1 << " ";
  ss << quat.q2 << " ";
  ss << quat.q3 << " ";
  ss << quat.q4;

  return ss.str();
}

std::string vtkTrackingRecord
::toMatrixString()
{
  // Get the matrix form of this tracking record
  vtkSmartPointer< vtkMatrix4x4 > m = this->GetMatrix();

  std::stringstream ss;
  ss << m->GetElement( 0, 0) << " ";
  ss << m->GetElement( 0, 1) << " ";
  ss << m->GetElement( 0, 2) << " ";
  ss << m->GetElement( 0, 3) << " ";

  ss << m->GetElement( 1, 0) << " ";
  ss << m->GetElement( 1, 1) << " ";
  ss << m->GetElement( 1, 2) << " ";
  ss << m->GetElement( 1, 3) << " ";

  ss << m->GetElement( 2, 0) << " ";
  ss << m->GetElement( 2, 1) << " ";
  ss << m->GetElement( 2, 2) << " ";
  ss << m->GetElement( 2, 3) << " ";

  ss << m->GetElement( 3, 0) << " ";
  ss << m->GetElement( 3, 1) << " ";
  ss << m->GetElement( 3, 2) << " ";
  ss << m->GetElement( 3, 3) << " ";

  return ss.str();
}


void vtkTrackingRecord
::fromString( std::string s )
{

  std::stringstream ss( s );
  
  double x, y, z;
  ss >> x; ss >> y; ss >> z;

  double q1, q2, q3, q4;
  ss >> q1; ss >> q2; ss >> q3; ss >> q4;

  trans.x = x;
  trans.y = y;
  trans.z = z;
  quat.q1 = q1;
  quat.q2 = q2;
  quat.q3 = q3;
  quat.q4 = q4;

}

void vtkTrackingRecord
::fromMatrixString( std::string s )
{

  std::stringstream ss( s );
  
  double e00; ss >> e00; double e01; ss >> e01; double e02; ss >> e02; double e03; ss >> e03;
  double e10; ss >> e10; double e11; ss >> e11; double e12; ss >> e12; double e13; ss >> e13;
  double e20; ss >> e20; double e21; ss >> e21; double e22; ss >> e22; double e23; ss >> e23;
  double e30; ss >> e30; double e31; ss >> e31; double e32; ss >> e32; double e33; ss >> e33;
  
  vtkSmartPointer< vtkMatrix4x4 > m = vtkSmartPointer< vtkMatrix4x4 >::New();
  
  m->SetElement( 0, 0, e00 );
  m->SetElement( 0, 1, e01 );
  m->SetElement( 0, 2, e02 );
  m->SetElement( 0, 3, e03 );
  
  m->SetElement( 1, 0, e10 );
  m->SetElement( 1, 1, e11 );
  m->SetElement( 1, 2, e12 );
  m->SetElement( 1, 3, e13 );
  
  m->SetElement( 2, 0, e20 );
  m->SetElement( 2, 1, e21 );
  m->SetElement( 2, 2, e22 );
  m->SetElement( 2, 3, e23 );
  
  m->SetElement( 3, 0, e30 );
  m->SetElement( 3, 1, e31 );
  m->SetElement( 3, 2, e32 );
  m->SetElement( 3, 3, e33 );
  
  this->SetMatrix( m );
}