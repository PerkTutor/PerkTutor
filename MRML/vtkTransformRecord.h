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
#include "vtkMatrix4x4.h"

// TransformRecorder includes
#include "vtkLogRecord.h"

#include "vtkSlicerTransformRecorderModuleMRMLExport.h"



class VTK_SLICER_TRANSFORMRECORDER_MODULE_MRML_EXPORT
vtkTransformRecord : public vtkLogRecord
{
public:
  vtkTypeMacro( vtkTransformRecord, vtkLogRecord );

  // Standard MRML node methods  
  static vtkTransformRecord *New();  
  
protected:

  // Constructor/desctructor
  vtkTransformRecord();
  virtual ~vtkTransformRecord();  
  
public:

  void Copy( vtkLogRecord* otherRecord );
  
  void SetTransformMatrix( vtkMatrix4x4* newMatrix4x4 );
  void SetTransformMatrix( double* newMatrixDouble );
  void SetTransformMatrix( std::string newMatrixString );
  
  void GetTransformMatrix( vtkMatrix4x4* matrix4x4 );
  void GetTransformMatrix( double* matrixDouble );
  std::string GetTransformMatrix();

  void SetDeviceName( std::string newDeviceName );
  std::string GetDeviceName();

  std::string ToXMLString( vtkIndent indent );
  void FromXMLElement( vtkXMLDataElement* element );


protected:
  
  std::string DeviceName;
  std::string TransformMatrix; // Store as string for read/write efficiency
  
};  

#endif
