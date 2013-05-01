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

// Standard includes
#include <ctime>
#include <iostream>
#include <utility>
#include <vector>

// VTK includes
#include "vtkMRMLModelNode.h"
#include "vtkTransform.h"
#include "vtkMRMLNode.h"
#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLLinearTransformNode.h"

// TransformRecorder includes
#include "vtkSlicerTransformRecorderModuleMRMLExport.h"

// Includes from this module
#include "vtkMRMLTransformBufferNode.h"


class VTK_SLICER_TRANSFORMRECORDER_MODULE_MRML_EXPORT
vtkMRMLTransformRecorderNode : public vtkMRMLNode
{
public:
 
  vtkTypeMacro( vtkMRMLTransformRecorderNode, vtkMRMLNode );
  
  // Standard MRML node methods
  
  static vtkMRMLTransformRecorderNode *New();
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "TransformRecorder"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );
  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  
protected:

  // Constructor/desctructor

  vtkMRMLTransformRecorderNode();
  virtual ~vtkMRMLTransformRecorderNode();
  vtkMRMLTransformRecorderNode ( const vtkMRMLTransformRecorderNode& );
  void operator=( const vtkMRMLTransformRecorderNode& );
  
  
  // Reference to observed transform nodes.
  
public:
  void AddObservedTransformNode( const char* TransformNodeID );
  void RemoveObservedTransformNode( const char* TransformNodeID );
  void ClearObservedTranformNodes();
  vtkMRMLLinearTransformNode* GetObservedTransformNode( const char* TransformNodeID );
  bool IsObservedTransformNode( const char* TransformNodeID );
  void AddObservedTransformNodesFromStoredNames();

protected:
  std::vector< std::string > StoredTransformNodeNames;
  std::vector< char* > ObservedTransformNodeIDs;
  std::vector< vtkMRMLLinearTransformNode* > ObservedTransformNodes;
  
public:

  void SetRecording( bool newRecording );
  bool GetRecording();
  void Clear();

  void GetCurrentTimestamp( int &sec, int &nsec );
  double GetCurrentTimestamp();

  void SetFileName( std::string newFileName );
  std::string GetFileName();
  void SaveToFile( std::string fileName );
  
  double GetTotalTime();
  double GetTotalPath();
  double GetTotalPathInside();

  void AddMessage( std::string message, double time = -1 );
  void AddTransform( const char* TransformNodeID ); 
  vtkMRMLTransformBufferNode* TransformBuffer;
  
protected:  

  std::string fileName;
  
  // Set a zero timestamp in the constructor using the system clock.  
  clock_t Clock0;  

  bool Recording;
  bool NeedleInside;
  
  double TotalNeedlePath;
  double TotalNeedlePathInside;

  vtkTransform* LastNeedleTransform;
  double LastNeedleTime;
  
};  

#endif
