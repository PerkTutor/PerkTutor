/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformRecorderNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkLogRecordBuffer_h
#define __vtkLogRecordBuffer_h

// Standard includes
#include <ctime>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>
#include <cmath>

// VTK includes
#include "vtkCommand.h"
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"
#include "vtkXMLDataParser.h"
#include "vtkSmartPointer.h"


// TransformRecorder includes
#include "vtkSlicerTransformRecorderModuleMRMLExport.h"

// Includes from this module
#include "vtkLogRecord.h"


// This class should be able to store a buffer of either transform records or message records
// There is no restriction on having transforms from different tools in the same buffer, but the vtkMRMLTransformBufferNode, will maintain separate vtkLogRecordBuffers for each tool
class VTK_SLICER_TRANSFORMRECORDER_MODULE_MRML_EXPORT
vtkLogRecordBuffer : public vtkObject
{
public:
  vtkTypeMacro( vtkLogRecordBuffer, vtkObject );

  // Standard MRML node methods  
  static vtkLogRecordBuffer *New();  
  
protected:

  // Constructor/desctructor
  vtkLogRecordBuffer();
  virtual ~vtkLogRecordBuffer();  
  
  
public:

  void Copy( vtkLogRecordBuffer* otherBuffer );

  void Concatenate( vtkLogRecordBuffer* catBuffer );
  
  int AddRecord( vtkLogRecord* newRecord ); // Return the index of the added location

  bool RemoveRecord( int index ); // Return whether the remove was successful
  
  vtkLogRecord* GetRecord( int index );
  vtkLogRecord* GetCurrentRecord();
  
  vtkLogRecord* GetRecordAtTime( double time );
  
  int GetNumRecords();

  double GetMinimumTime();
  double GetMaximumTime();
  double GetTotalTime();

  void Clear();

  std::string ToXMLString();
  void FromXMLElement( vtkXMLDataElement* element );


protected:

  int GetPriorRecordIndex( double time );
  int GetClosestRecordIndex( double time );
  
  std::vector< vtkSmartPointer< vtkLogRecord > > Records;

};  

#endif
