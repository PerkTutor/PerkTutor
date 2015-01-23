/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTableStorageNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.6 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLTableStorageNode.h"
#include "vtkMRMLTableNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkDelimitedTextReader.h>
#include <vtkDelimitedTextWriter.h>
#include <vtkTable.h>
#include <vtkStringArray.h>
#include <vtkNew.h>
#include <vtksys/SystemTools.hxx>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTableStorageNode);

//----------------------------------------------------------------------------
vtkMRMLTableStorageNode::vtkMRMLTableStorageNode()
{
}

//----------------------------------------------------------------------------
vtkMRMLTableStorageNode::~vtkMRMLTableStorageNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLTableStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  vtkMRMLStorageNode::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
bool vtkMRMLTableStorageNode::CanReadInReferenceNode(vtkMRMLNode *refNode)
{
  return refNode->IsA("vtkMRMLTableNode");
}

//----------------------------------------------------------------------------
int vtkMRMLTableStorageNode::ReadDataInternal(vtkMRMLNode *refNode)
{
  std::string fullName = this->GetFullNameFromFileName();

  if (fullName == std::string(""))
    {
    vtkErrorMacro("vtkMRMLTableStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLTableNode *tableNode =
    vtkMRMLTableNode::SafeDownCast(refNode);

  if (tableNode == NULL)
    {
    vtkErrorMacro("ReadData: unable to cast input node " << refNode->GetID()
                  << " to a table (measurement) node");
    return 0;
    }

  // check that the file exists
  if (vtksys::SystemTools::FileExists(fullName.c_str()) == false)
    {
    vtkErrorMacro("ReadDataInternal: model file '" << fullName.c_str() << "' not found.");
    return 0;
    }

  // compute file prefix
  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);
  if( extension.empty() )
    {
    vtkErrorMacro("ReadData: no file extension specified: " << fullName.c_str());
    return 0;
    }

  vtkDebugMacro("ReadDataInternal: extension = " << extension.c_str());

  int result = 1;
  try
    {
    if ( extension == std::string(".csv") )
      {
      vtkNew<vtkDelimitedTextReader> reader;
      reader->SetFileName(fullName.c_str());
      reader->SetFieldDelimiterCharacters(",");
      reader->SetHaveHeaders(false);
      reader->SetDetectNumericColumns(true);
      reader->Update();
      tableNode->SetTable(reader->GetOutput());
      }
    else
      {
      vtkDebugMacro("Cannot read table file '" << fullName.c_str() << "' (extension = " << extension.c_str() << ")");
      return 0;
      }
    }
  catch (...)
    {
    result = 0;
    }

  return result;
}

//----------------------------------------------------------------------------
int vtkMRMLTableStorageNode::WriteDataInternal(vtkMRMLNode *refNode)
{
  if (this->GetFileName() == NULL)
    {
    vtkErrorMacro("WriteData: file name is not set");
    return 0;
    }

  std::string fullName = this->GetFullNameFromFileName();
  if (fullName == std::string(""))
    {
    vtkErrorMacro("vtkMRMLTableStorageNode: File name not specified");
    return 0;
    }

  // cast the input node
  vtkMRMLTableNode *tableNode =
    vtkMRMLTableNode::SafeDownCast(refNode);

  if (tableNode == NULL)
    {
    vtkErrorMacro("WriteData: unable to cast input node " << refNode->GetID() << " to a known table node");
    return 0;
    }

  std::string extension = vtkMRMLStorageNode::GetLowercaseExtensionFromFileName(fullName);

  int result = 1;
  if (extension == ".csv")
    {
    vtkNew<vtkDelimitedTextWriter> writer;
    writer->SetFileName(fullName.c_str());
    writer->SetInputData( tableNode->GetTable() );
    try
      {
      writer->Write();
      }
    catch (...)
      {
      result = 0;
      }
    }
  else
    {
    result = 0;
    vtkErrorMacro( << "No file extension recognized: " << fullName.c_str() );
    }

  return result;
}

//----------------------------------------------------------------------------
void vtkMRMLTableStorageNode::InitializeSupportedReadFileTypes()
{
  this->SupportedReadFileTypes->InsertNextValue("Measurement CSV (.csv)");
  this->SupportedReadFileTypes->InsertNextValue("Text (.txt)");
}

//----------------------------------------------------------------------------
void vtkMRMLTableStorageNode::InitializeSupportedWriteFileTypes()
{
  this->SupportedWriteFileTypes->InsertNextValue("Measurement CSV (.csv)");
  this->SupportedWriteFileTypes->InsertNextValue("Text (.txt)");
}

//----------------------------------------------------------------------------
const char* vtkMRMLTableStorageNode::GetDefaultWriteFileExtension()
{
  return "csv";
}
