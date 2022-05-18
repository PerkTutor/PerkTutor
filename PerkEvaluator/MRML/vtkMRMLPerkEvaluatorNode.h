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
#include "vtkMRMLSequenceBrowserNode.h"
#include "vtkMRMLTableNode.h"


class VTK_SLICER_PERKEVALUATOR_MODULE_MRML_EXPORT
vtkMRMLPerkEvaluatorNode : public vtkMRMLNode
{
public:
  vtkTypeMacro( vtkMRMLPerkEvaluatorNode, vtkMRMLNode );

  // Standard MRML node methods  
  static vtkMRMLPerkEvaluatorNode* New();  
  vtkMRMLNode* CreateNodeInstance() override;
  const char* GetNodeTagName() override { return "PerkEvaluator"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  void ReadXMLAttributes( const char** atts ) override;
  void WriteXML( ostream& of, int indent ) override;
  void Copy( vtkMRMLNode *node ) override;

  
protected:

  // Constructor/desctructor
  vtkMRMLPerkEvaluatorNode();
  virtual ~vtkMRMLPerkEvaluatorNode();
  vtkMRMLPerkEvaluatorNode ( const vtkMRMLPerkEvaluatorNode& ); // Required to prevent linking error
  void operator=( const vtkMRMLPerkEvaluatorNode& ); // Required to prevent linking error
  
  
public:

  // Whether to update the parameters when the sequence browser node is changed
  bool GetAutoUpdateMeasurementRange();
  void SetAutoUpdateMeasurementRange( bool update );

  // Whether to compute task-specific metrics (in addition to computing overall metrics)
  bool GetComputeTaskSpecificMetrics();
  void SetComputeTaskSpecificMetrics( bool compute );

  // Whether to ignore transforms that are not proxy nodes or children of proxy nodes (that is their ToWorld transform is unchanged by browsing the sequence)
  bool GetIgnoreIrrelevantTransforms();
  void SetIgnoreIrrelevantTransforms( bool ignore );

  // Analysis start/end times (note: these are relative times)
  double GetMarkBegin();
  void SetMarkBegin( double newBegin );
  
  double GetMarkEnd();
  void SetMarkEnd( double newEnd );

  // Enumerate all of the possible needle orientations
  enum NeedleOrientationEnum{ PlusX, MinusX, PlusY, MinusY, PlusZ, MinusZ };  
  // Needle orientation
  void GetNeedleOrientation( double needleOrientation[ 3 ] ); // This is the direction in which the needle points
  NeedleOrientationEnum GetNeedleOrientation();
  void SetNeedleOrientation( NeedleOrientationEnum needleOrietation );
  
  // Metric nodes
  void AddMetricInstanceID( std::string metricInstanceID );
  void RemoveMetricInstanceID( std::string metricInstanceID );
  std::vector< std::string > GetMetricInstanceIDs();
  bool IsMetricInstanceID( std::string metricInstanceID );
  void SetMetricInstanceIDs( std::vector< std::string > metricInstanceIDs );

  bool GetRealTimeProcessing();
  void SetRealTimeProcessing( bool newRealTimeProcessing );

  // Analysis state
  // -1 means analysis is halted
  // Other values indicate the progress of the analysis (on 0%-100%)
  int GetAnalysisState();
  void SetAnalysisState( int newAnalysisState );


  // Reference to sequence browser node and to metrics node
  std::string GetNodeReferenceIDString( std::string referenceRole );

  vtkMRMLSequenceBrowserNode* GetTrackedSequenceBrowserNode();
  std::string GetTrackedSequenceBrowserNodeID();
  void SetTrackedSequenceBrowserNodeID( std::string newTrackedSequenceBrowserNodeID );

  void UpdateMeasurementRange();

  vtkMRMLTableNode* GetMetricsTableNode();
  std::string GetMetricsTableID();
  void SetMetricsTableID( std::string newMetricsTableID );

  // Pass along sequence browser node events
  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData );
  enum
  {
    TransformRealTimeAddedEvent = vtkCommand::UserEvent + 1,
    RealTimeProcessingStartedEvent,
    AnalysisStateUpdatedEvent,
  };
  
  
  // Deprecated
  // Attributes to facilitate loading from "old-style" scenes
  // These are public so that they can be accessed from the logic without maintaining infrastructure
  std::map< std::string, std::string > TransformRoleMap; // From transform node names to roles
  std::map< std::string, std::string > AnatomyNodeMap; // From roles to anatomy node names
  std::string MetricsDirectory; // From roles to anatomy node names
  
protected:

  bool AutoUpdateMeasurementRange;
  bool ComputeTaskSpecificMetrics;
  bool IgnoreIrrelevantTransforms;

  double MarkBegin;
  double MarkEnd;
  
  NeedleOrientationEnum NeedleOrientation;

  int AnalysisState;
  bool RealTimeProcessing;

};  

#endif
