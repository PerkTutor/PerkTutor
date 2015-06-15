/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformRecorderNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkLogRecord_h
#define __vtkLogRecord_h

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
vtkLogRecord : public vtkObject
{
public:
  vtkTypeMacro( vtkLogRecord, vtkObject );

  // Standard MRML node methods  
  static vtkLogRecord *New();  
  
protected:

  // Constructor/desctructor
  vtkLogRecord();
  virtual ~vtkLogRecord();
  
  
public:

  void Copy( vtkLogRecord* otherRecord );
  
  void SetTimeStampSec( int newTimeStampSec );
  int GetTimeStampSec();

  void SetTimeStampNSec( int newTimeStampNSec );
  int GetTimeStampNSec();

  void SetTime( double time );
  double GetTime();

  virtual std::string ToXMLString( vtkIndent indent );
  virtual void FromXMLElement( vtkXMLDataElement* element );


protected:

  long int TimeStampSec; 
  int TimeStampNSec;
  
};  

#endif
