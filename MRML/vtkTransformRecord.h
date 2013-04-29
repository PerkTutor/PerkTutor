/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformRecorderNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkTransformRecord_h
#define __vtkTransformRecord_h

// Standard includes
#include <ctime>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

// VTK includes
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"
#include "vtkXMLDataElement.h"

// TransformRecorder includes
#include "vtkSlicerTransformRecorderModuleMRMLExport.h"



class VTK_SLICER_TRANSFORMRECORDER_MODULE_MRML_EXPORT
vtkTransformRecord : public vtkObject
{
public:
  vtkTypeMacro( vtkTransformRecord, vtkObject );

  // Standard MRML node methods  
  static vtkTransformRecord *New();  
  
protected:

  // Constructor/desctructor
  vtkTransformRecord();
  virtual ~vtkTransformRecord();  
  
public:

  vtkTransformRecord* DeepCopy();
  
  void SetTransform( std::string newTransform );
  std::string GetTransform();

  void SetDeviceName( std::string newDeviceName );
  std::string GetDeviceName();

  void SetTimeStampSec( int newTimeStampSec );
  int GetTimeStampSec();

  void SetTimeStampNSec( int newTimeStampNSec );
  int GetTimeStampNSec();

  void SetTime( double time );
  double GetTime();

  std::string ToXMLString( int indent = 2 );
  void FromXMLElement( vtkXMLDataElement* element );


private:
  
  std::string DeviceName;
  std::string Transform;
  long int TimeStampSec; 
  int TimeStampNSec;
  
};  

#endif
