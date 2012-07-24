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

#include <ctime>
#include <iostream>
#include <utility>
#include <vector>


#include "vtkMRMLModelNode.h"
#include "vtkTransform.h"
#include "vtkMRMLNode.h"
#include "vtkMRML.h"
#include "vtkMRMLScene.h"

// TransformRecorder includes
#include "vtkSlicerTransformRecorderModuleMRMLExport.h"

class vtkActor;
class vtkImageActor;
class vtkMatrix4x4;
class vtkPolyData;
class vtkRenderer;
class vtkTransform;


class vtkImageData;
class vtkMRMLLinearTransformNode;
class vtkMRMLModelNode;
class vtkMRMLViewNode;
class vtkMRMLVolumeNode;

  /**
 * Struct to store a recorded transform.
 */
class TransformRecord
{
public:
  std::string DeviceName;
  std::string Transform;
  long int TimeStampSec; // UNIX time, rounded down. Seconds from 1970 Jan 1 00:00, UTC.
  int TimeStampNSec;     // Nanoseconds from TimeStampSec to the real timestamp.
};



class MessageRecord
{
public:
  std::string Message;
  long int TimeStampSec;
  int TimeStampNSec;
};
//ETX


class
VTK_SLICER_TRANSFORMRECORDER_MODULE_MRML_EXPORT
vtkMRMLTransformRecorderNode
: public vtkMRMLNode
{
public:
  
  //BTX
  // Events.
  enum {
    TransformChangedEvent = 201001,
    RecordingStartEvent   = 200901,
    RecordingStopEvent    = 200902
  };
  //ETX
  
  
    // Standard MRML node methods
  
  static vtkMRMLTransformRecorderNode *New();
  vtkTypeMacro( vtkMRMLTransformRecorderNode, vtkMRMLNode );
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "TransformRecorder"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );
  virtual void UpdateScene( vtkMRMLScene * );
  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  
  virtual void UpdateReferenceID( const char *oldID, const char *newID );
  void UpdateReferences();
  void StartReceiveServer();
  void StopReceiveServer();
  
  
protected:

    // Constructor/desctructor

  vtkMRMLTransformRecorderNode();
  virtual ~vtkMRMLTransformRecorderNode();
  vtkMRMLTransformRecorderNode ( const vtkMRMLTransformRecorderNode& );
  void operator=( const vtkMRMLTransformRecorderNode& );

  void RemoveMRMLObservers();
  
  
    // Reference to observed transform nodes.
  
public:
  void AddObservedTransformNode( const char* TransformNodeID );
  void RemoveObservedTransformNode( const char* TransformNodeID );
  void ClearObservedTranformNodes();
  vtkMRMLLinearTransformNode* GetObservedTransformNode( const char* TransformNodeID );
protected:
  std::vector< char* > ObservedTransformNodeIDs;
  std::vector< vtkMRMLLinearTransformNode* > ObservedTransformNodes;
  
  
public:
  unsigned int GetTransformsBufferSize();
  unsigned int GetMessagesBufferSize();
  double GetTotalTime();
  double GetTotalPath();
  double GetTotalPathInside();
  
  vtkGetMacro( Recording, bool );
  void SetRecording( bool newState );
  
  //BTX
  void SetTransformSelections( std::vector< int > selections );
  void SetLogFileName( std::string fileName );
  void SaveIntoFile( std::string fileName );
  std::string GetLogFileName();
  void CustomMessage( std::string message );
  //ETX
  
  void UpdateFileFromBuffer();
  void ClearBuffer();
  
  
protected:
  
  void AddNewTransform( const char* TransformNodeID );
  
  
  //BTX
  std::vector< int > TransformSelections;
  
  std::string LogFileName;
  std::vector< TransformRecord > TransformsBuffer;
  std::vector< MessageRecord > MessagesBuffer;
  //ETX
  
  bool Recording;
  bool NeedleInside;
  
  
    // Time.
    // Set a zero timestamp in the constructor using the system clock.
  
  clock_t Clock0;
  
  double IGTLTimeOffsetSeconds;  // Adding this to the IGTL timestamp synchronizes it with the clock.
  bool IGTLTimeSynchronized;
  
  double TotalNeedlePath;
  double TotalNeedlePathInside;
  vtkTransform* LastNeedleTransform;
  double LastNeedleTime;
  bool Active;
  
};  

#endif
