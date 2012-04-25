/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformRecorderNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLTransformRecorderNode_h
#define __vtkMRMLTransformRecorderNode_h

#include "vtkMRMLModelNode.h"

// TransformRecorder includes
#include "vtkSlicerTransformRecorderModuleMRMLExport.h"

class VTK_SLICER_TRANSFORMRECORDER_MODULE_MRML_EXPORT vtkMRMLTransformRecorderNode : public vtkMRMLModelNode
{
public:
  //static vtkMRMLTransformRecorderNode *New();
  void PrintSelf(ostream& os, vtkIndent indent);
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
                                   unsigned long /*event*/, 
                                   void * /*callData*/ );
protected:
  vtkMRMLTransformRecorderNode();
  ~vtkMRMLTransformRecorderNode();
  vtkMRMLTransformRecorderNode(const vtkMRMLTransformRecorderNode&);
  void operator=(const vtkMRMLTransformRecorderNode&);

};

#endif
