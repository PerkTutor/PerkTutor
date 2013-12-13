/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformRecorderNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLTransformBufferNode_h
#define __vtkMRMLTransformBufferNode_h

// Standard includes
#include <ctime>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>

// VTK includes
#include "vtkMRMLModelNode.h"
#include "vtkTransform.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorableNode.h"
#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"
#include "vtkXMLDataParser.h"


// TransformRecorder includes
#include "vtkSlicerTransformRecorderModuleMRMLExport.h"

// Includes from this module
#include "vtkMRMLTransformBufferStorageNode.h"
#include "vtkTransformRecord.h"
#include "vtkMessageRecord.h"



class VTK_SLICER_TRANSFORMRECORDER_MODULE_MRML_EXPORT
vtkMRMLTransformBufferNode : public vtkMRMLStorableNode
{
public:
  vtkTypeMacro( vtkMRMLTransformBufferNode, vtkMRMLStorableNode );

  // Standard MRML node methods  
  static vtkMRMLTransformBufferNode* New();  
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "TransformBuffer"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );

  // To use the storage node
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode() { return vtkMRMLTransformBufferStorageNode::New(); };
  bool GetModifiedSinceRead() { return ( this->GetMTime() > this->GetStoredTime() ); };
  virtual void UpdateScene( vtkMRMLScene *scene ) { Superclass::UpdateScene(scene); };
  
protected:

  // Constructor/desctructor
  vtkMRMLTransformBufferNode();
  virtual ~vtkMRMLTransformBufferNode();
  vtkMRMLTransformBufferNode ( const vtkMRMLTransformBufferNode& ); // Required to prevent linking error
  void operator=( const vtkMRMLTransformBufferNode& ); // Required to prevent linking error
  
  
public:
  
  void AddTransform( vtkTransformRecord* newTransform );
  void AddMessage( vtkMessageRecord* newMessage );

  void RemoveTransformAt( int index );
  void RemoveMessageAt( int index );
  void RemoveTransformsByName( std::string name );
  void RemoveMessagesByName( std::string name );

  vtkTransformRecord* GetTransformAt( int index );
  vtkTransformRecord* GetCurrentTransform();
  vtkTransformRecord* GetTransformByName( std::string name );
  vtkMessageRecord* GetMessageAt( int index );
  vtkMessageRecord* GetCurrentMessage();
  vtkMessageRecord* GetMessageByName( std::string name );

  vtkTransformRecord* GetTransformAtTime( double time );
  vtkMessageRecord* GetMessageAtTime( double time );

  int GetNumTransforms();
  int GetNumMessages();

  double GetMinimumTime();
  double GetMaximumTime();
  double GetTotalTime();

  void Clear();
  void ClearTransforms();
  void ClearMessages();

  void AddActiveTransform( std::string name );
  void RemoveActiveTransform( std::string name );
  std::vector<std::string> GetActiveTransforms();
  void SetActiveTransforms( std::vector<std::string> names );
  void SetActiveTransformsFromBuffer();

  std::vector<vtkMRMLTransformBufferNode*> SplitBufferByName();
  vtkMRMLTransformBufferNode* GetBufferByName( std::string name );

  std::string ToXMLString();
  void FromXMLElement( vtkXMLDataElement* element );

  // Keep track of what has been updated
  unsigned long TransformsStatus;
  unsigned long MessagesStatus;
  unsigned long ActiveTransformsStatus;


private:

  int GetPriorTransformIndex( double time );
  int GetClosestTransformIndex( double time );
  int GetPriorMessageIndex( double time );
  int GetClosestMessageIndex( double time );
  
  std::vector<vtkTransformRecord*> transforms;
  std::vector<vtkMessageRecord*> messages;

  std::vector<std::string> activeTransforms;

};  

#endif
