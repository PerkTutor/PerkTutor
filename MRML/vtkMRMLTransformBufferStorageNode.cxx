/*=Auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransformStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkMRMLTransformBufferStorageNode.h"
#include "vtkMRMLTransformBufferNode.h"


// Standard MRML Node Methods ------------------------------------------------------------

vtkMRMLTransformBufferStorageNode* vtkMRMLTransformBufferStorageNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLTransformBufferStorageNode" );
  if( ret )
    {
      return ( vtkMRMLTransformBufferStorageNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTransformBufferStorageNode();
}


vtkMRMLNode* vtkMRMLTransformBufferStorageNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLTransformBufferStorageNode" );
  if( ret )
    {
      return ( vtkMRMLTransformBufferStorageNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTransformBufferStorageNode();
}



void vtkMRMLTransformBufferStorageNode
::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLTransformBufferStorageNode
::vtkMRMLTransformBufferStorageNode()
{
}


vtkMRMLTransformBufferStorageNode
::~vtkMRMLTransformBufferStorageNode()
{
}

// Storage node specific methods ----------------------------------------------------------------------------
bool vtkMRMLTransformBufferStorageNode
::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA( "vtkMRMLTransformBufferNode" );
}


void vtkMRMLTransformBufferStorageNode
::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Transform Buffer (.xml)");
}


const char* vtkMRMLTransformBufferStorageNode
::GetDefaultWriteFileExtension()
{
  return "xml";
}



// Read and Write methods ----------------------------------------------------------------------------
int vtkMRMLTransformBufferStorageNode
::ReadDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLTransformBufferNode* bufferNode = vtkMRMLTransformBufferNode::SafeDownCast( refNode );

  std::string fullName = this->GetFullNameFromFileName(); 
  if ( fullName == std::string( "" ) ) 
  {
    vtkErrorMacro("vtkMRMLTransformBufferNode: File name not specified");
    return 0;
  }

  // Clear the current buffer prior to importing
  bufferNode->Clear();

  vtkXMLDataParser* parser = vtkXMLDataParser::New();
  parser->SetFileName( fullName.c_str() );
  parser->Parse();

  bufferNode->FromXMLElement( parser->GetRootElement() );
  bufferNode->SetActiveTransformsFromBuffer();

  // The buffer name should already be specified
  // The scene should already be populated with the desired transforms

  parser->Delete();

  return 1;
}

//----------------------------------------------------------------------------
int vtkMRMLTransformBufferStorageNode
::WriteDataInternal(vtkMRMLNode *refNode)
{
  vtkMRMLTransformBufferNode* bufferNode = vtkMRMLTransformBufferNode::SafeDownCast( refNode );

  std::string fullName =  this->GetFullNameFromFileName();
  if ( fullName == std::string( "" ) ) 
  {
    vtkErrorMacro("vtkMRMLTransformBufferNode: File name not specified");
    return 0;
  }

  std::ofstream output( fullName.c_str() );
  
  if ( ! output.is_open() )
  {
    vtkErrorMacro( "Record file could not be opened!" );
    return 0;
  }

  output << bufferNode->ToXMLString();

  output.close();

  return 1;
}


