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
// #include "RecordType.h"
// #include "ToolData.h"

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

//-------------------------------------------------------------------------
// MRML Node classes
//-------------------------------------------------------------------------


class
VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkMRMLWorkflowSegmentationNode
: public vtkMRMLNode
{
public:
  
  //Enumeration of events
  //BTX
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
  
  
protected:

  // Constructor/desctructor methods
  vtkMRMLWorkflowSegmentationNode();
  virtual ~vtkMRMLWorkflowSegmentationNode();
  vtkMRMLWorkflowSegmentationNode ( const vtkMRMLWorkflowSegmentationNode& );
  void operator=( const vtkMRMLWorkflowSegmentationNode& );

  void RemoveMRMLObservers();
    
  
public:
  
  // Reference to observed transform nodes.
  void AddObservedTransformNode( const char* TransformNodeID );
  void RemoveObservedTransformNode( const char* TransformNodeID );
  void ClearObservedTranformNodes();
  vtkMRMLLinearTransformNode* GetObservedTransformNode( const char* TransformNodeID );

protected:

  // Reference to observed transform nodes
  std::vector< char* > ObservedTransformNodeIDs;
  std::vector< vtkMRMLLinearTransformNode* > ObservedTransformNodes;
  
  
public:

  // Whether we are recording
  bool GetRecording();
  void SetRecording( bool newState );

  // File name setters and getters
  std::string GetTrackingLogFileName();
  void SetTrackingLogFileName( std::string name );
  std::string GetSegmentationLogFileName();
  void SetSegmentationLogFileName( std::string name );
  std::string GetProcedureDefinitionFileName();
  void SetProcedureDefinitionFileName( std::string name );
  std::string GetInputParameterFileName();
  void SetInputParameterFileName( std::string name );
  std::string GetTrainingParameterFileName();
  void SetTrainingParameterFileName( std::string name );

  
  // Setters for saving the scene
  //BTX
  void SetTransformSelections( std::vector< int > selections );
  //ETX
  
  // File IO methods
  void SaveTrackingLog();
  void SaveSegmentation();
  void SaveTrainingParameters();
  void ImportProcedureDefinition();
  void ImportInputParameters();
  void ImportTrainingParameters();
  void ImportAvailableData();
  
  // Get the current time stamp sec, nanosec
  void GetTimestamp( int &sec, int &nsec );
  double GetTimestamp();
  
 
  //Observe a new transform
  void AddNewTransform( const char* TransformNodeID ); 
  void AddNewTransform( TransformRecord rec );

  
protected:
  
  // Variables associated with recording
  //BTX
  std::vector< int > TransformSelections;  
  //ETX

  // Input/output files
  std::string TrackingLogFileName;
  std::string SegmentationLogFileName;
  std::string ProcedureDefinitionFileName;
  std::string InputParameterFileName;
  std::string TrainingParameterFileName;
 
  // Active recording
  bool Recording;
  
  // Time.
  // Set a zero timestamp in the constructor using the system clock.  
  clock_t Clock0;
  
  // Clock synchronization
  double IGTLTimeOffsetSeconds;  // Adding this to the IGTL timestamp synchronizes it with the clock.
  bool IGTLTimeSynchronized;

  // Keep track of the last recorded transform to avoid repeats
  vtkTransform* LastNeedleTransform;
  double LastNeedleTime;
  

public:
  ToolCollection toolCollection;
  
};  

#endif
