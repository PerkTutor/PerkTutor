
#include "vtkWorkflowTool.h"

vtkStandardNewMacro( vtkWorkflowTool );


vtkWorkflowTool
::vtkWorkflowTool()
{
  this->Name = "";
  this->Defined = false;
  this->Inputted = false;
  this->Trained = false;

  this->Procedure = vtkWorkflowProcedure::New();
  this->Input = vtkWorkflowInput::New();
  this->Training = vtkWorkflowTraining::New();
  this->Buffer = vtkRecordBuffer::New();
}


vtkWorkflowTool
::~vtkWorkflowTool()
{
  this->Name = "";
  vtkDelete( this->Procedure );
  vtkDelete( this->Input );
  vtkDelete( this->Training );
  vtkDelete( this->Buffer );
}


vtkWorkflowTool* vtkWorkflowTool
::DeepCopy()
{
  vtkWorkflowTool* newWorkflowTool = vtkWorkflowTool::New();
  newWorkflowTool->Name = this->Name;
  newWorkflowTool->Defined = this->Defined;
  newWorkflowTool->Inputted = this->Inputted;
  newWorkflowTool->Trained = this->Trained;
  newWorkflowTool->Procedure = vtkDeleteAssign( newWorkflowTool->Procedure, this->Procedure->DeepCopy() );
  newWorkflowTool->Input = vtkDeleteAssign( newWorkflowTool->Input, this->Input->DeepCopy() );
  newWorkflowTool->Training = vtkDeleteAssign( newWorkflowTool->Training, this->Training->DeepCopy() );
  newWorkflowTool->Buffer = vtkDeleteAssign( newWorkflowTool->Buffer, this->Buffer->DeepCopy() );
  return newWorkflowTool;
}


std::string vtkWorkflowTool
::ProcedureToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "  <Tool Name=\"" << this->Name << "\" >" << std::endl;
  xmlstring << this->Procedure->ToXMLString() << std::endl;
  xmlstring << "  </Tool>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowTool
::ProcedureFromXMLElement( vtkXMLDataElement* element )
{
  if ( strcmp( element->GetName(), "Tool" ) == 0 && strcmp( element->GetAttribute( "Name" ), this->Name.c_str() ) == 0 )
  {
    this->Procedure->FromXMLElement( element );
	this->Defined = true;
  }
}


std::string vtkWorkflowTool
::InputToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "  <Tool Name=\"" << this->Name << "\" >" << std::endl;
  xmlstring << this->Input->ToXMLString() << std::endl;
  xmlstring << "  </Tool>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowTool
::InputFromXMLElement( vtkXMLDataElement* element )
{
  if ( strcmp( element->GetName(), "Tool" ) == 0 && strcmp( element->GetAttribute( "Name" ), this->Name.c_str() ) == 0 )
  {
    this->Input->FromXMLElement( element );
	this->Inputted = true;
  }
}


std::string vtkWorkflowTool
::TrainingToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "  <Tool Name=\"" << this->Name << "\" >" << std::endl;
  xmlstring << this->Training->ToXMLString();
  xmlstring << "  </Tool>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowTool
::TrainingFromXMLElement( vtkXMLDataElement* element )
{
  if ( strcmp( element->GetName(), "Tool" ) == 0 && strcmp( element->GetAttribute( "Name" ), this->Name.c_str() ) == 0 )
  {
    this->Training->FromXMLElement( element );
	this->Trained = true;
  }
}