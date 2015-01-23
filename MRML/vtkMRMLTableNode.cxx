/*=auto=========================================================================

Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTableNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLTableNode.h"
#include "vtkMRMLTableStorageNode.h"

// VTK includes
#include <vtkTable.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkCxxSetObjectMacro(vtkMRMLTableNode, Table, vtkTable)

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTableNode);

//----------------------------------------------------------------------------
vtkMRMLTableNode::vtkMRMLTableNode()
{
  this->Table = vtkTable::New();

  this->HideFromEditorsOff();
}

//----------------------------------------------------------------------------
vtkMRMLTableNode::~vtkMRMLTableNode()
{
  if (this->Table)
    {
    this->Table->Delete();
    this->Table = NULL;
    }
}


//----------------------------------------------------------------------------
void vtkMRMLTableNode::WriteXML(ostream& of, int nIndent)
{

  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);
}


//----------------------------------------------------------------------------
void vtkMRMLTableNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  vtkMRMLNode::ReadXMLAttributes(atts);

  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// 
void vtkMRMLTableNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();
  Superclass::Copy(anode);
  vtkMRMLTableNode *node = vtkMRMLTableNode::SafeDownCast(anode);
  if (node)
    {
    this->SetTable(node->GetTable());
    }
  this->EndModify(disabledModify);
}


//----------------------------------------------------------------------------
void vtkMRMLTableNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  //if (this->TargetPlanList && this->TargetPlanList == vtkMRMLFiducialListNode::SafeDownCast(caller) &&
  //  event ==  vtkCommand::ModifiedEvent)
  //  {
  //  //this->InvokeEvent(vtkMRMLVolumeNode::ImageDataModifiedEvent, NULL);
  //  //this->UpdateFromMRML();
  //  return;
  //  }
  //
  //if (this->TargetCompletedList && this->TargetCompletedList == vtkMRMLFiducialListNode::SafeDownCast(caller) &&
  //  event ==  vtkCommand::ModifiedEvent)
  //  {
  //  //this->InvokeEvent(vtkMRMLVolumeNode::ImageDataModifiedEvent, NULL);
  //  //this->UpdateFromMRML();
  //  return;
  //  }

  return;
}


//----------------------------------------------------------------------------
void vtkMRMLTableNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "\nTable Data:";
  if (this->GetTable())
    {
    os << "\n";
    this->GetTable()->PrintSelf(os, indent.GetNextIndent());
    }
  else
    {
    os << "none\n";
    }
}

//---------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLTableNode::CreateDefaultStorageNode()
{
  return vtkMRMLStorageNode::SafeDownCast(vtkMRMLTableStorageNode::New());
}

