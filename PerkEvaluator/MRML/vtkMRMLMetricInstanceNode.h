/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTransformRecorderNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLMetricInstanceNode_h
#define __vtkMRMLMetricInstanceNode_h

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



// PerkEvaluator includes
#include "vtkMRMLMetricScriptNode.h"
#include "vtkSlicerPerkEvaluatorModuleMRMLExport.h"



class VTK_SLICER_PERKEVALUATOR_MODULE_MRML_EXPORT
vtkMRMLMetricInstanceNode : public vtkMRMLTransformableNode
{
public:
  vtkTypeMacro( vtkMRMLMetricInstanceNode, vtkMRMLTransformableNode );

  // Standard MRML node methods  
  static vtkMRMLMetricInstanceNode* New();  
  virtual vtkMRMLNode* CreateNodeInstance();
  virtual const char* GetNodeTagName() { return "MetricInstance"; };
  void PrintSelf( ostream& os, vtkIndent indent );
  virtual void ReadXMLAttributes( const char** atts );
  virtual void WriteXML( ostream& of, int indent );
  virtual void Copy( vtkMRMLNode *node );

  
protected:

  // Constructor/desctructor
  vtkMRMLMetricInstanceNode();
  virtual ~vtkMRMLMetricInstanceNode();
  vtkMRMLMetricInstanceNode ( const vtkMRMLMetricInstanceNode& ); // Required to prevent linking error
  void operator=( const vtkMRMLMetricInstanceNode& ); // Required to prevent linking error
  
  
public:

  // Transform and Anatomy roles
  // Note: No functions should take this enum as a parameter, otherwise they cannot be Python wrapped
  // Instead, take int as a parameter (this should be the case for all functions, so we don't need to cast enum <-> int)
  enum RoleTypeEnum
  { 
    TransformRole = 0, 
    AnatomyRole,
    NumberOfRoleTypes,
  };

  vtkMRMLNode* GetRoleNode( std::string role, /*vtkMRMLMetricInstanceNode::RoleTypeEnum*/ int roleType );
  std::string GetRoleID( std::string role, /*vtkMRMLMetricInstanceNode::RoleTypeEnum*/ int roleType );
  void SetRoleID( std::string nodeID, std::string role, /*vtkMRMLMetricInstanceNode::RoleTypeEnum*/ int roleType );
  std::string GetCombinedRoleString();


  
  // Script for the metric
  vtkMRMLMetricScriptNode* GetAssociatedMetricScriptNode();
  std::string GetAssociatedMetricScriptID();
  void SetAssociatedMetricScriptID( std::string newAssociatedMetricScriptID );
  
protected:

  static std::string GetFullReferenceRoleName( std::string role, /*vtkMRMLMetricInstanceNode::RoleTypeEnum*/ int roleType );
  std::string GetNodeReferenceIDString( std::string referenceRole );
  void UpdateNodeName();
 
};  

#endif
