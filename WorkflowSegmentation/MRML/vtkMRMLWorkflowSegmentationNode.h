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
#include "vtkCommand.h"

#include "vtkMRMLSequenceBrowserNode.h"

// WorkflowSegmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkMRMLWorkflowToolNode.h"

class
VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkMRMLWorkflowSegmentationNode : public vtkMRMLNode
{
public:
  vtkTypeMacro( vtkMRMLWorkflowSegmentationNode, vtkMRMLNode );
  
  // Standard MRML node methods  
  static vtkMRMLWorkflowSegmentationNode *New();  

  vtkMRMLNode* CreateNodeInstance() override;
  const char* GetNodeTagName() override { return "WorkflowSegmentation"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  void ReadXMLAttributes( const char** atts ) override;
  void WriteXML( ostream& of, int indent ) override;
  void Copy( vtkMRMLNode *node ) override;
  
protected:

  // Constructor/desctructor methods
  vtkMRMLWorkflowSegmentationNode();
  virtual ~vtkMRMLWorkflowSegmentationNode();
  vtkMRMLWorkflowSegmentationNode ( const vtkMRMLWorkflowSegmentationNode& );
  void operator=( const vtkMRMLWorkflowSegmentationNode& );
 
  
public:

  std::string GetNodeReferenceIDString( std::string referenceRole );

  // Reference to selected transform buffer
  vtkMRMLSequenceBrowserNode* GetTrackedSequenceBrowserNode();
  std::string GetTrackedSequenceBrowserNodeID();
  void SetTrackedSequenceBrowserNodeID( std::string newTrackedSequenceBrowserNodeID );

  bool GetRealTimeProcessing();
  void SetRealTimeProcessing( bool newRealTimeProcessing );

  // Management of references to tools
  void AddToolID( std::string toolID );
  void RemoveToolID( std::string toolID );
  std::vector< std::string > GetToolIDs();
  bool IsToolID( std::string toolID );
  void SetToolIDs( std::vector< std::string > toolIDs );

  // Propagate the modified event from any of the tools
  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData ) override;
  enum
  {
    TransformRealTimeAddedEvent = vtkCommand::UserEvent + 1,
    RealTimeProcessingStartedEvent,
  };

protected:

  bool RealTimeProcessing;

};  

#endif
