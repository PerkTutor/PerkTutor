/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLWorkflowInputStorageNode_h
#define __vtkMRMLWorkflowInputStorageNode_h

// Standard includes
#include <ctime>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

//VTK includes
#include "vtkMRMLStorageNode.h"
#include "vtkStringArray.h"

#include <vtkXMLDataParser.h>

// TransformRecorder includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"


/// Storage nodes has methods to read/write workflow input to/from disk.
class VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkMRMLWorkflowInputStorageNode : public vtkMRMLStorageNode
{
public:
  vtkTypeMacro( vtkMRMLWorkflowInputStorageNode, vtkMRMLStorageNode );

  // Standard MRML node methods  
  static vtkMRMLWorkflowInputStorageNode* New();
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName()  { return "WorkflowInputStorage"; };
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
  vtkMRMLWorkflowInputStorageNode();
  ~vtkMRMLWorkflowInputStorageNode();
  vtkMRMLWorkflowInputStorageNode(const vtkMRMLWorkflowInputStorageNode&);
  void operator=(const vtkMRMLWorkflowInputStorageNode&);


  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// Write data from a referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode);
  
};

#endif
