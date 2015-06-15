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
#include <cmath>
#include <limits>

// VTK includes
#include "vtkCommand.h"
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
#include "vtkNew.h"
#include "vtkIntArray.h"

// MRML Includes
#include "vtkMRMLLinearTransformNode.h"


// TransformRecorder includes
#include "vtkSlicerTransformRecorderModuleMRMLExport.h"

// Includes from this module
#include "vtkMRMLTransformBufferStorageNode.h"
#include "vtkTransformRecord.h"
#include "vtkMessageRecord.h"
#include "vtkLogRecord.h"
#include "vtkLogRecordBuffer.h"

#include "PerkTutorCommon.h"



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

  void Concatenate( vtkMRMLTransformBufferNode* catBuffer );
  
  void AddTransform( vtkTransformRecord* newTransform );
  void AddMessage( vtkMessageRecord* newMessage );

  void RemoveTransform( int index, std::string transformName );
  void RemoveTransformsByName( std::string name );
  
  void RemoveMessage( int index );
  void RemoveMessagesByName( std::string name );

  vtkTransformRecord* GetTransformAtIndex( int index, std::string transformName );
  vtkTransformRecord* GetCurrentTransform( std::string transformName );

  vtkMessageRecord* GetMessageAtIndex( int index );
  vtkMessageRecord* GetCurrentMessage();

  vtkTransformRecord* GetTransformAtTime( double time, std::string transformName );
  vtkMessageRecord* GetMessageAtTime( double time );

  vtkLogRecordBuffer* GetTransformRecordBuffer( std::string transformName );

  std::vector< std::string > GetAllRecordedTransformNames();

  int GetNumTransforms( std::string transformName );
  int GetNumTransforms();
  int GetNumMessages();

  double GetMinimumTime();
  double GetMaximumTime();
  double GetTotalTime();

  void Clear();
  void ClearTransforms();
  void ClearMessages();

  void AddActiveTransformID( std::string transformID );
  void RemoveActiveTransformID( std::string transformID );
  std::vector< std::string > GetActiveTransformIDs();
  bool IsActiveTransformID( std::string transformID );
  void SetActiveTransformIDs( std::vector< std::string > transformIDs );

  void StartRecording();
  void StopRecording();
  bool GetRecording();
  double GetCurrentTimestamp();

  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );

  std::string ToXMLString( vtkIndent indent );
  void FromXMLElement( vtkXMLDataElement* element );

  // The events that the class should invole
  typedef std::pair< std::string, int > TransformEventDataType;
  enum
  {
    TransformAddedEvent = vtkCommand::UserEvent + 1,
    TransformRemovedEvent,
    MessageAddedEvent,
    MessageRemovedEvent,
    ActiveTransformAddedEvent,
    ActiveTransformRemovedEvent,
  };


protected:

  void GetCombinedTransformRecordBuffer( vtkLogRecordBuffer* combinedTransformRecordBuffer );
  
  std::map< std::string, vtkSmartPointer< vtkLogRecordBuffer > > TransformRecordBuffers;
  vtkSmartPointer< vtkLogRecordBuffer > MessageRecordBuffer;

  bool RecordingState;
  double Clock0;

};  

#endif
