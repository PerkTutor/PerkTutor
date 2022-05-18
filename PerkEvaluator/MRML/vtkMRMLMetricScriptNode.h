/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformRecorderNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLMetricScriptNode_h
#define __vtkMRMLMetricScriptNode_h

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
#include "vtkMRMLStorableNode.h"

// PerkEvaluator includes
#include "vtkMRMLMetricScriptStorageNode.h"
#include "vtkSlicerPerkEvaluatorModuleMRMLExport.h"



class VTK_SLICER_PERKEVALUATOR_MODULE_MRML_EXPORT
vtkMRMLMetricScriptNode : public vtkMRMLStorableNode
{
public:
  vtkTypeMacro( vtkMRMLMetricScriptNode, vtkMRMLStorableNode );

  // Standard MRML node methods  
  static vtkMRMLMetricScriptNode* New();  
  vtkMRMLNode* CreateNodeInstance() override;
  const char* GetNodeTagName() override { return "MetricScript"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  void ReadXMLAttributes( const char** atts ) override;
  void WriteXML( ostream& of, int indent ) override;
  void Copy( vtkMRMLNode *node ) override;

  // To use the storage node
  vtkMRMLStorageNode* CreateDefaultStorageNode() override { return vtkMRMLMetricScriptStorageNode::New(); };
  bool GetModifiedSinceRead() { return ( this->GetMTime() > this->GetStoredTime() ); };
  void UpdateScene( vtkMRMLScene *scene ) override { vtkMRMLStorableNode::UpdateScene(scene); };

  
protected:

  // Constructor/desctructor
  vtkMRMLMetricScriptNode();
  virtual ~vtkMRMLMetricScriptNode();
  vtkMRMLMetricScriptNode ( const vtkMRMLMetricScriptNode& ); // Required to prevent linking error
  void operator=( const vtkMRMLMetricScriptNode& ); // Required to prevent linking error
  
  
public:

  // C++ interface for each function
  // Note: These are just convenience methods for the static functions
  // The "heavy lifting" is performed with the PythonMetricsCalculator module

  /* TODO: Remove?
  std::string GetMetricName();
  std::string GetMetricUnit();
  
  std::vector< std::string > GetAllTransformRoles();
  std::map< std::string, std::string > GetAllAnatomyRoles();
  */

  std::string GetPythonSourceCode();
  void SetPythonSourceCode( std::string newPythonSourceCode );

  // Compare metric scripts
  bool IsEqual( vtkMRMLMetricScriptNode* msNode );

  // Instances of the metrics
  bool IsAssociatedMetricInstanceID( std::string associatedMetricInstanceID ); // TODO: Is this ok?

  enum
  {
    PythonSourceCodeChangedEvent = vtkCommand::UserEvent + 1,
  };

  
protected:

  std::string PythonSourceCode;
 
};  

#endif
