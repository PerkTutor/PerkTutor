/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLWorkflowTrainingStorageNode_h
#define __vtkMRMLWorkflowTrainingStorageNode_h

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
vtkMRMLWorkflowTrainingStorageNode : public vtkMRMLStorageNode
{
public:
  vtkTypeMacro( vtkMRMLWorkflowTrainingStorageNode, vtkMRMLStorageNode );

  // Standard MRML node methods  
  static vtkMRMLWorkflowTrainingStorageNode* New();
  vtkMRMLNode* CreateNodeInstance() override;
  const char* GetNodeTagName() override { return "WorkflowTrainingStorage"; };
  void PrintSelf(ostream& os, vtkIndent indent);
  // No need for special read/write/copy

  // Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;
  // Return a default file extension for writting
  const char* GetDefaultWriteFileExtension() override;

  /// Support only transform buffer nodes
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

protected:
  // Constructor/deconstructor
  vtkMRMLWorkflowTrainingStorageNode();
  ~vtkMRMLWorkflowTrainingStorageNode();
  vtkMRMLWorkflowTrainingStorageNode(const vtkMRMLWorkflowTrainingStorageNode&);
  void operator=(const vtkMRMLWorkflowTrainingStorageNode&);


  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  /// Write data from a referenced node
  int WriteDataInternal(vtkMRMLNode *refNode) override;
  
};

#endif
