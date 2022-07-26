/*=Auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkMRMLWorkflowInputStorageNode.h"
#include "vtkMRMLWorkflowInputNode.h"


// Standard MRML Node Methods ------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLWorkflowInputStorageNode);


void vtkMRMLWorkflowInputStorageNode
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLWorkflowInputStorageNode
::vtkMRMLWorkflowInputStorageNode()
{
}


vtkMRMLWorkflowInputStorageNode
::~vtkMRMLWorkflowInputStorageNode()
{
}

// Storage node specific methods ----------------------------------------------------------------------------
bool vtkMRMLWorkflowInputStorageNode
::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA( "vtkMRMLWorkflowInputNode" );
}


void vtkMRMLWorkflowInputStorageNode
::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue( "Workflow Input (.xml)" );
}


const char* vtkMRMLWorkflowInputStorageNode
::GetDefaultWriteFileExtension()
{
  return "xml";
}



// Read and Write methods ----------------------------------------------------------------------------
int vtkMRMLWorkflowInputStorageNode
::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLWorkflowInputNode* workflowInputNode = vtkMRMLWorkflowInputNode::SafeDownCast( refNode );

  std::string fullName = this->GetFullNameFromFileName(); 
  if ( fullName == std::string( "" ) ) 
  {
    vtkErrorMacro( "vtkMRMLWorkflowInputNode: File name not specified" );
    return 0;
  }

  vtkSmartPointer< vtkXMLDataParser > parser = vtkSmartPointer< vtkXMLDataParser >::New();
  parser->SetFileName( fullName.c_str() );
  parser->Parse();

  workflowInputNode->FromXMLElement( parser->GetRootElement() );

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLWorkflowInputStorageNode
::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLWorkflowInputNode* workflowInputNode = vtkMRMLWorkflowInputNode::SafeDownCast( refNode );

  std::string fullName = this->GetFullNameFromFileName();
  if ( fullName == std::string( "" ) ) 
  {
    vtkErrorMacro( "vtkMRMLWorkflowInputNode: File name not specified" );
    return 0;
  }

  std::ofstream output( fullName.c_str() );
  
  if ( ! output.is_open() )
  {
    vtkErrorMacro( "Record file could not be opened!" );
    return 0;
  }

  output << workflowInputNode->ToXMLString( vtkIndent() );

  output.close();

  return 1;
}


