/*=Auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkMRMLMetricScriptStorageNode.h"
#include "vtkMRMLMetricScriptNode.h"


// Standard MRML Node Methods ------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMetricScriptStorageNode);

void vtkMRMLMetricScriptStorageNode
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLMetricScriptStorageNode
::vtkMRMLMetricScriptStorageNode()
{
}


vtkMRMLMetricScriptStorageNode
::~vtkMRMLMetricScriptStorageNode()
{
}

// Storage node specific methods ----------------------------------------------------------------------------
bool vtkMRMLMetricScriptStorageNode
::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA( "vtkMRMLMetricScriptNode" );
}


void vtkMRMLMetricScriptStorageNode
::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue( "Python Metric Script (*.py)" );
}


const char* vtkMRMLMetricScriptStorageNode
::GetDefaultWriteFileExtension()
{
  return "py";
}



// Read and Write methods ----------------------------------------------------------------------------
int vtkMRMLMetricScriptStorageNode
::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLMetricScriptNode* msNode = vtkMRMLMetricScriptNode::SafeDownCast( refNode );

  std::string fullName = this->GetFullNameFromFileName(); 
  if ( fullName == std::string( "" ) ) 
  {
    vtkErrorMacro( "vtkMRMLMetricScriptNode: File name not specified" );
    return 0;
  }

  std::ifstream inFileStream( fullName.c_str() );
  
  if ( ! inFileStream.is_open() )
  {
    vtkErrorMacro( "vtkMRMLMetricScriptNode: Record file could not be opened!" );
    return 0;
  }
  
  std::stringstream sourceCodeStream;
  sourceCodeStream << inFileStream.rdbuf();

  msNode->SetPythonSourceCode( sourceCodeStream.str() );
  
  inFileStream.close();

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLMetricScriptStorageNode
::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLMetricScriptNode* msNode = vtkMRMLMetricScriptNode::SafeDownCast( refNode );

  std::string fullName = this->GetFullNameFromFileName();
  if ( fullName == std::string( "" ) ) 
  {
    vtkErrorMacro( "vtkMRMLMetricScriptNode: File name not specified" );
    return 0;
  }

  std::ofstream outFileStream( fullName.c_str() );
  
  if ( ! outFileStream.is_open() )
  {
    vtkErrorMacro( "vtkMRMLMetricScriptNode: Record file could not be opened!" );
    return 0;
  }

  outFileStream << msNode->GetPythonSourceCode();

  outFileStream.close();

  return 1;
}


