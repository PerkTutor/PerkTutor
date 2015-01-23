/*=auto=========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLTableNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLTableNode_h
#define __vtkMRMLTableNode_h

#include <string>
#include <vector>

#include "vtkSlicerPerkEvaluatorModuleMRMLExport.h"
#include "vtkMRMLStorableNode.h"

// MRML Includes
class vtkMRMLStorageNode;

// VTK Includes
class vtkTable;

/// \brief MRML node to represent a table object
///
/// It is meant to be a replacement to the vtkMRMLDoubleArrayNode
/// to store anything table like.
class VTK_SLICER_PERKEVALUATOR_MODULE_MRML_EXPORT 
vtkMRMLTableNode : public vtkMRMLStorableNode
{
public:
  static vtkMRMLTableNode *New();
  vtkTypeMacro(vtkMRMLTableNode,vtkMRMLStorableNode);

  void PrintSelf(ostream& os, vtkIndent indent);

  //----------------------------------------------------------------
  /// Standard methods for MRML nodes
  //----------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName()
    {return "Table";};

  ///
  /// Method to propagate events generated in mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //----------------------------------------------------------------
  /// Get and Set Macros
  //----------------------------------------------------------------
  virtual void SetTable(vtkTable*);
  vtkGetObjectMacro ( Table, vtkTable );

  ///
  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  //----------------------------------------------------------------
  /// Constructor and destroctor
  //----------------------------------------------------------------
 protected:
  vtkMRMLTableNode();
  ~vtkMRMLTableNode();
  vtkMRMLTableNode(const vtkMRMLTableNode&);
  void operator=(const vtkMRMLTableNode&);


 protected:
  //----------------------------------------------------------------
  /// Data
  //----------------------------------------------------------------

  vtkTable* Table;
};

#endif

