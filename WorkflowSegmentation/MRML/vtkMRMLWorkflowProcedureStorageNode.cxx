/*=Auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkMRMLWorkflowProcedureStorageNode.h"
#include "vtkMRMLWorkflowProcedureNode.h"


// Standard MRML Node Methods ------------------------------------------------------------

vtkMRMLWorkflowProcedureStorageNode* vtkMRMLWorkflowProcedureStorageNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowProcedureStorageNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowProcedureStorageNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowProcedureStorageNode();
}


vtkMRMLNode* vtkMRMLWorkflowProcedureStorageNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowProcedureStorageNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowProcedureStorageNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowProcedureStorageNode();
}



void vtkMRMLWorkflowProcedureStorageNode
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLWorkflowProcedureStorageNode
::vtkMRMLWorkflowProcedureStorageNode()
{
}


vtkMRMLWorkflowProcedureStorageNode
::~vtkMRMLWorkflowProcedureStorageNode()
{
}

// Storage node specific methods ----------------------------------------------------------------------------
bool vtkMRMLWorkflowProcedureStorageNode
::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA( "vtkMRMLWorkflowProcedureNode" );
}


void vtkMRMLWorkflowProcedureStorageNode
::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue( "Workflow Procedure (.xml)" );
}


const char* vtkMRMLWorkflowProcedureStorageNode
::GetDefaultWriteFileExtension()
{
  return "xml";
}



// Read and Write methods ----------------------------------------------------------------------------
int vtkMRMLWorkflowProcedureStorageNode
::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLWorkflowProcedureNode* workflowProcedureNode = vtkMRMLWorkflowProcedureNode::SafeDownCast( refNode );

  std::string fullName = this->GetFullNameFromFileName(); 
  if ( fullName == std::string( "" ) ) 
  {
    vtkErrorMacro( "vtkMRMLWorkflowProcedureNode: File name not specified" );
    return 0;
  }

  vtkNew<vtkXMLDataParser> parser;
  parser->SetFileName( fullName.c_str() );
  parser->Parse();

  workflowProcedureNode->FromXMLElement( parser->GetRootElement() );

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLWorkflowProcedureStorageNode
::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLWorkflowProcedureNode* workflowProcedureNode = vtkMRMLWorkflowProcedureNode::SafeDownCast( refNode );

  std::string fullName = this->GetFullNameFromFileName();
  if ( fullName == std::string( "" ) ) 
  {
    vtkErrorMacro( "vtkMRMLWorkflowProcedureNode: File name not specified" );
    return 0;
  }

  std::ofstream output( fullName.c_str() );
  
  if ( ! output.is_open() )
  {
    vtkErrorMacro( "Record file could not be opened!" );
    return 0;
  }

  output << workflowProcedureNode->ToXMLString( vtkIndent() );

  output.close();

  return 1;
}


