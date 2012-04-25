/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformRecorderNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// VTK includes
#include <vtkCleanPolyData.h>
#include <vtkCommand.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkExtractSelectedPolyDataIds.h>
#include <vtkIdTypeArray.h>
#include <vtkInformation.h>
#include <vtkObjectFactory.h>
#include <vtkPlanes.h>
#include <vtkSelection.h>
#include <vtkSelectionNode.h>

// TransformRecorder MRML includes
#include "vtkMRMLTransformRecorderNode.h"


// MRML includes
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLAnnotationNode.h>
#include <vtkMRMLAnnotationROINode.h>

// STD includes
#include <math.h>
#include <vector>
#include <algorithm>


//------------------------------------------------------------------------------
//vtkMRMLNodeNewMacro(vtkMRMLTransformRecorderNode);

//-----------------------------------------------------------------------------
vtkMRMLTransformRecorderNode::vtkMRMLTransformRecorderNode()
{

}

//-----------------------------------------------------------------------------
vtkMRMLTransformRecorderNode::~vtkMRMLTransformRecorderNode()
{

}

//----------------------------------------------------------------------------
void vtkMRMLTransformRecorderNode::PrintSelf(ostream& os, vtkIndent indent)
{
  
  Superclass::PrintSelf(os,indent);

}

//---------------------------------------------------------------------------
void vtkMRMLTransformRecorderNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, 
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  return;
} 