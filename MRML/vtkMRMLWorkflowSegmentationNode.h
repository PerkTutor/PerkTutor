/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLWorkflowSegmentationNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLWorkflowSegmentationNode_h
#define __vtkMRMLWorkflowSegmentationNode_h

#include <ctime>
#include <iostream>
#include <utility>
#include <vector>


#include "vtkMRMLModelNode.h"
#include "vtkTransform.h"
#include "vtkMRMLNode.h"
#include "vtkMRML.h"
#include "vtkMRMLScene.h"

// WorkflowSegmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"

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
  long int TimeStampSec; 
  int TimeStampNSec;     // Nanoseconds from TimeStampSec to the real timestamp.
  std::string Task;
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
VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkMRMLWorkflowSegmentationNode
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
  
  static vtkMRMLWorkflowSegmentationNode *New();
  vtkTypeMacro( vtkMRMLWorkflowSegmentationNode, vtkMRMLNode );
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "WorkflowSegmentation"; };
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

  vtkMRMLWorkflowSegmentationNode();
  virtual ~vtkMRMLWorkflowSegmentationNode();
  vtkMRMLWorkflowSegmentationNode ( const vtkMRMLWorkflowSegmentationNode& );
  void operator=( const vtkMRMLWorkflowSegmentationNode& );

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
  std::string GetCurrentTask();
  
  vtkGetMacro( Recording, bool );
  void SetRecording( bool newState );
  
  //BTX
  void SetTransformSelections( std::vector< int > selections );
  void SetLogFileName( std::string fileName );
  void SaveIntoFile( std::string fileName );
  std::string GetLogFileName();
  void CustomMessage( std::string message, int sec = -1, int nsec = -1 );
  //ETX
  
  void UpdateFileFromBuffer();
  void ImportTrainingData( std::string dirName );
  void ImportInputParameters( std::string fileName );
  void TrainSegmentationAlgorithm();
  void ClearBuffer();
  
  void GetTimestamp( int &sec, int &nsec );
  
  
protected:
  
  void AddNewTransform( const char* TransformNodeID );
  
  
  //BTX
  std::vector< int > TransformSelections;
  
  std::string LogFileName;
  std::vector< TransformRecord > TransformsBuffer;
  std::vector< MessageRecord > MessagesBuffer;
  //ETX
  
  bool Recording;  
  
    // Time.
    // Set a zero timestamp in the constructor using the system clock.
  
  clock_t Clock0;
  
  double IGTLTimeOffsetSeconds;  // Adding this to the IGTL timestamp synchronizes it with the clock.
  bool IGTLTimeSynchronized;

  vtkTransform* LastNeedleTransform;
  double LastNeedleTime;
  bool Active;
  
};  

#endif
