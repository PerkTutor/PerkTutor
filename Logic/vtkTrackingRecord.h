
#ifndef VTKTRACKINGRECORD_H
#define VTKTRACKINGRECORD_H

#include "vtkMatrix4x4.h"
#include "vtkSmartPointer.h"
#include "vtkObject.h"
#include "vtkSlicerWorkflowSegmentationModuleLogicExport.h"

#include <vector>
#include <iostream>


// Class representing the translation part
class TranslationXYZ
{
public:
	double x, y, z;
};


// Class representing the quaternion rotation part
class RotationQuaternion
{
public:
	double q1, q2, q3, q4;
};


// Class representing a particular record for tracking data
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_LOGIC_EXPORT
  vtkTrackingRecord : public vtkObject
{
public:

  static vtkTrackingRecord *New();

  vtkTrackingRecord* DeepCopy();

  TranslationXYZ GetTranslation();  
  void SetTranslation( TranslationXYZ tr );

  RotationQuaternion GetRotation();
  void SetRotation( RotationQuaternion q );

  std::vector<double> GetVector();
  void SetVector( std::vector<double> v );

  vtkSmartPointer<vtkMatrix4x4> GetMatrix();
  void SetMatrix( vtkSmartPointer<vtkMatrix4x4> m );

  double GetTime();
  void SetTime( double t );

  std::string GetDeviceName();
  void SetDeviceName( std::string d );

  std::string toString();
  std::string toMatrixString();
  void fromString( std::string s );
  void fromMatrixString( std::string s );



  vtkTrackingRecord();
  ~vtkTrackingRecord();

private:

  static const int TRACKINGRECORD_SIZE = 7;

  TranslationXYZ trans;
  RotationQuaternion quat;
  double time;
  std::string deviceName;

};

#endif
