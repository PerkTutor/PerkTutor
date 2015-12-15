/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformRecorderNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLPerkEvaluatorNode_h
#define __vtkMRMLPerkEvaluatorNode_h

// Standard includes
#include <ctime>
#include <iostream>
#include <sstream>
#include <utility>
#include <vector>
#include <cmath>

// VTK includes
#include "vtkCommand.h"
#include "vtkTransform.h"
#include "vtkObject.h"
#include "vtkObjectBase.h"
#include "vtkObjectFactory.h"


// Slicer includes
#include "vtkMRMLTransformableNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLLinearTransformNode.h"


// PerkEvaluator includes
#include "vtkSlicerPerkEvaluatorModuleMRMLExport.h"


// Includes from other modules
#include "vtkMRMLTransformBufferNode.h"
#include "vtkMRMLTableNode.h"


class VTK_SLICER_PERKEVALUATOR_MODULE_MRML_EXPORT
vtkMRMLPerkEvaluatorNode : public vtkMRMLTransformableNode
{
public:
  vtkTypeMacro( vtkMRMLPerkEvaluatorNode, vtkMRMLTransformableNode );

  // Standard MRML node methods  
  static vtkMRMLPerkEvaluatorNode* New();  
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "PerkEvaluator"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );

  
protected:

  // Constructor/desctructor
  vtkMRMLPerkEvaluatorNode();
  virtual ~vtkMRMLPerkEvaluatorNode();
  vtkMRMLPerkEvaluatorNode ( const vtkMRMLPerkEvaluatorNode& ); // Required to prevent linking error
  void operator=( const vtkMRMLPerkEvaluatorNode& ); // Required to prevent linking error
  
  
public:

  // Whether to update the parameters when the transform buffer node is changed
  bool GetAutoUpdateMeasurementRange();
  void SetAutoUpdateMeasurementRange( bool update );

  bool GetAutoUpdateTransformRoles();
  void SetAutoUpdateTransformRoles( bool update );

  // Analysis start/end times (note: these are relative times)
  double GetMarkBegin();
  void SetMarkBegin( double newBegin );
  
  double GetMarkEnd();
  void SetMarkEnd( double newEnd );

  // Enumerate all of the possible needle orientations
  enum NeedleOrientationEnum{ PlusX, MinusX, PlusY, MinusY, PlusZ, MinusZ };  
  // Needle orientation
  void GetNeedleBase( double needleBase[ 4 ] );
  NeedleOrientationEnum GetNeedleOrientation();
  void SetNeedleOrientation( NeedleOrientationEnum needleOrietation );
  
  // Metric nodes
  bool IsMetricScriptID( std::string metricScriptID );
  void SetMetricScriptIDs( std::vector< std::string > metricScriptIDs );

  // Playback time
  double GetPlaybackTime();
  void SetPlaybackTime( double newPlaybackTime, bool analysis = false );

  bool GetRealTimeProcessing();
  void SetRealTimeProcessing( bool newRealTimeProcessing );
  

  // Getters/setters associated with roles
  std::string GetTransformRole( std::string transformNodeName );
  std::string GetFirstTransformNodeName( std::string transformRole );
  void SetTransformRole( std::string transformNodeName, std::string newTransformRole );

  std::string GetAnatomyNodeName( std::string anatomyRole );
  std::string GetFirstAnatomyRole( std::string anatomyNodeName );
  void SetAnatomyNodeName( std::string anatomyRole, std::string newAnatomyNodeName );


  // Reference to transform buffer and to metrics node
  std::string GetNodeReferenceIDString( std::string referenceRole );

  vtkMRMLTransformBufferNode* GetTransformBufferNode();
  std::string GetTransformBufferID();
  void SetTransformBufferID( std::string newTransformBufferID );

  vtkMRMLTableNode* GetMetricsTableNode();
  std::string GetMetricsTableID();
  void SetMetricsTableID( std::string newMetricsTableID );

  // Pass along transform buffer events
  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );
  enum
  {
    TransformRealTimeAddedEvent = vtkCommand::UserEvent + 1,
    RealTimeProcessingStartedEvent,
  };
  

  
protected:

/* To store:
TransformRoleMap
AnatomyRoleMap
MarkEnd
MarkBegin
NeedleOrientation
MetricsDirectory
PlaybackTime
*/

  bool AutoUpdateMeasurementRange;
  bool AutoUpdateTransformRoles;

  double MarkBegin;
  double MarkEnd;
  
  NeedleOrientationEnum NeedleOrientation;
  
  std::string MetricsDirectory;

  double PlaybackTime;

  bool RealTimeProcessing;

  std::map< std::string, std::string > TransformRoleMap; // From transform node names to roles
  std::map< std::string, std::string > AnatomyNodeMap; // From roles to anatomy node names
  
};  

#endif
