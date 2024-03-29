/*=Auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkMRMLWorkflowTrainingStorageNode.h"
#include "vtkMRMLWorkflowTrainingNode.h"


// Standard MRML Node Methods ------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLWorkflowTrainingStorageNode);

void vtkMRMLWorkflowTrainingStorageNode
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLWorkflowTrainingStorageNode
::vtkMRMLWorkflowTrainingStorageNode()
{
}


vtkMRMLWorkflowTrainingStorageNode
::~vtkMRMLWorkflowTrainingStorageNode()
{
}

// Storage node specific methods ----------------------------------------------------------------------------
bool vtkMRMLWorkflowTrainingStorageNode
::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA( "vtkMRMLWorkflowTrainingNode" );
}


void vtkMRMLWorkflowTrainingStorageNode
::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue( "Workflow Training (.xml)" );
}


const char* vtkMRMLWorkflowTrainingStorageNode
::GetDefaultWriteFileExtension()
{
  return "xml";
}



// Read and Write methods ----------------------------------------------------------------------------
int vtkMRMLWorkflowTrainingStorageNode
::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLWorkflowTrainingNode* workflowTrainingNode = vtkMRMLWorkflowTrainingNode::SafeDownCast( refNode );

  std::string fullName = this->GetFullNameFromFileName(); 
  if ( fullName == std::string( "" ) ) 
  {
    vtkErrorMacro( "vtkMRMLWorkflowTrainingNode: File name not specified" );
    return 0;
  }

  vtkNew<vtkXMLDataParser> parser;
  parser->SetFileName( fullName.c_str() );
  parser->Parse();

  workflowTrainingNode->FromXMLElement( parser->GetRootElement() );

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLWorkflowTrainingStorageNode
::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLWorkflowTrainingNode* workflowTrainingNode = vtkMRMLWorkflowTrainingNode::SafeDownCast( refNode );

  std::string fullName = this->GetFullNameFromFileName();
  if ( fullName == std::string( "" ) ) 
  {
    vtkErrorMacro( "vtkMRMLWorkflowTrainingNode: File name not specified" );
    return 0;
  }

  std::ofstream output( fullName.c_str() );
  
  if ( ! output.is_open() )
  {
    vtkErrorMacro( "Training file could not be opened!" );
    return 0;
  }

  output << workflowTrainingNode->ToXMLString( vtkIndent() );

  output.close();

  return 1;
}


