/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLTransformStorageNode_h
#define __vtkMRMLTransformStorageNode_h

// Standard includes
#include <ctime>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

//VTK includes
#include "vtkMRMLStorageNode.h"
#include "vtkStringArray.h"

// TransformRecorder includes
#include "vtkSlicerTransformRecorderModuleMRMLExport.h"


/// Storage nodes has methods to read/write transform bufferss to/from disk.
class VTK_SLICER_TRANSFORMRECORDER_MODULE_MRML_EXPORT
vtkMRMLTransformBufferStorageNode : public vtkMRMLStorageNode
{
public:
  vtkTypeMacro( vtkMRMLTransformBufferStorageNode, vtkMRMLStorageNode );

  // Standard MRML node methods  
  static vtkMRMLTransformBufferStorageNode* New();
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName()  { return "TransformBufferStorage"; };
  void PrintSelf(ostream& os, vtkIndent indent);
  // No need for special read/write/copy

  // Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();
  // Return a default file extension for writting
  virtual const char* GetDefaultWriteFileExtension();

  /// Support only transform buffer nodes
  virtual bool CanReadInReferenceNode(vtkMRMLNode* refNode);

protected:
  // Constructor/deconstructor
  vtkMRMLTransformBufferStorageNode();
  ~vtkMRMLTransformBufferStorageNode();
  vtkMRMLTransformBufferStorageNode(const vtkMRMLTransformBufferStorageNode&);
  void operator=(const vtkMRMLTransformBufferStorageNode&);


  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// Write data from a referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode);

};

#endif
