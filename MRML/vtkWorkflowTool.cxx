
#include "vtkWorkflowTool.h"

vtkStandardNewMacro( vtkWorkflowTool );


vtkWorkflowTool
::vtkWorkflowTool()
{
  this->Name = "";
  this->Defined = false;
  this->Inputted = false;
  this->Trained = false;
}


vtkWorkflowTool
::~vtkWorkflowTool()
{
  this->Name = "";
  this->Defined = false;
  this->Inputted = false;
  this->Trained = false;
}


std::string vtkWorkflowTool
::PerkProcedureToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "  <Tool Name=\"" << this->Name << "\" />" << std::endl;
  xmlstring << this->Procedure.ToXMLString() << std::endl;
  xmlstring << "  </Tool>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowTool
::PerkProcedureFromXMLElement( vtkXMLDataElement* element )
{
  if ( strcmp( element->GetAttribute( "Name" ), this->Name.c_str() ) == 0 )
  {
    this->Procedure.FromXMLElement( element );
	this->Defined = true;
  }
}


std::string vtkWorkflowTool
::InputParameterToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "  <Tool Name=\"" << this->Name << "\" />" << std::endl;
  xmlstring << this->Input.ToXMLString() << std::endl;
  xmlstring << "  </Tool>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowTool
::InputParameterFromXMLElement( vtkXMLDataElement* element )
{
  if ( strcmp( element->GetAttribute( "Name" ), this->Name.c_str() ) == 0 )
  {
    this->Input.FromXMLElement( element );
	this->Inputted = true;
  }
}


std::string vtkWorkflowTool
::TrainingParameterToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "  <Tool Name=\"" << this->Name << "\" />" << std::endl;
  xmlstring << this->Training.ToXMLString() << std::endl;
  xmlstring << "  </Tool>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowTool
::TrainingParameterFromXMLElement( vtkXMLDataElement* element )
{
  if ( strcmp( element->GetAttribute( "Name" ), this->Name.c_str() ) == 0 )
  {
    this->Training.FromXMLElement( element, this->Procedure, this->Input );
	this->Trained = true;
  }
}