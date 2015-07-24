/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformRecorderNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMessageRecord_h
#define __vtkMessageRecord_h

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
#include  "vtkLogRecord.h"

#include "vtkSlicerTransformRecorderModuleMRMLExport.h"



class VTK_SLICER_TRANSFORMRECORDER_MODULE_MRML_EXPORT
vtkMessageRecord : public vtkLogRecord
{
public:
  vtkTypeMacro( vtkMessageRecord, vtkLogRecord );

  // Standard MRML node methods  
  static vtkMessageRecord *New();  
  
protected:

  // Constructor/desctructor
  vtkMessageRecord();
  virtual ~vtkMessageRecord();
  
  
public:

  void Copy( vtkLogRecord* otherRecord );
  
  void SetMessageString( std::string newMessageString );
  std::string GetMessageString();

  std::string ToXMLString( vtkIndent );
  void FromXMLElement( vtkXMLDataElement* element );


protected:
  
  // I would like to call this "Message", but there is a Windows conflict with the "GetMessage" function
  std::string MessageString;
  
};  

#endif
