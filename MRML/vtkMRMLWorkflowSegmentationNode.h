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
#include "vtkXMLDataElement.h"

// WorkflowSegmentation includes
#include "vtkSlicerWorkflowSegmentationModuleMRMLExport.h"
#include "vtkWorkflowToolCollection.h"

class
VTK_SLICER_WORKFLOWSEGMENTATION_MODULE_MRML_EXPORT
vtkMRMLWorkflowSegmentationNode
: public vtkMRMLNode
{
public:
  vtkTypeMacro( vtkMRMLWorkflowSegmentationNode, vtkMRMLNode );
  
  // Standard MRML node methods  
  static vtkMRMLWorkflowSegmentationNode *New();  

  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "WorkflowSegmentation"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );
  
protected:

  // Constructor/desctructor methods
  vtkMRMLWorkflowSegmentationNode();
  virtual ~vtkMRMLWorkflowSegmentationNode();
  vtkMRMLWorkflowSegmentationNode ( const vtkMRMLWorkflowSegmentationNode& );
  void operator=( const vtkMRMLWorkflowSegmentationNode& );
 
  
public:

  // File name setters and getters
  std::string GetWorkflowProcedureFileName();
  void SetWorkflowProcedureFileName( std::string newWorkflowProcedureFileName );
  std::string GetWorkflowInputFileName();
  void SetWorkflowInputFileName( std::string newWorkflowInputFileName );
  std::string GetWorkflowTrainingFileName();
  void SetWorkflowTrainingFileName( std::string newWorkflowTrainingFileName );
  
  // File IO methods
  void SaveWorkflowTraining( std::string newWorkflowTrainingFileName = "" );
  void ImportWorkflowProcedure( std::string newWorkflowProcedureFileName = "" );
  void ImportWorkflowInput( std::string newWorkflowInputFileName = "" );
  void ImportWorkflowTraining( std::string newWorkflowTrainingFileName = "" );
  void ImportAllWorkflowData();

  vtkWorkflowTool* GetCompletionTool( vtkWorkflowTool* tool );

protected:

  // Input/output files
  std::string WorkflowProcedureFileName;
  std::string WorkflowInputFileName;
  std::string WorkflowTrainingFileName;

  vtkXMLDataParser* Parser;
  vtkXMLDataElement* ParseXMLFile( std::string fileName );

public:

  vtkWorkflowToolCollection* ToolCollection;
  vtkWorkflowToolCollection* ToolCompletion;

};  

#endif
