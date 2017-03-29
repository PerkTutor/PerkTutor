
#include "vtkWorkflowTask.h"

vtkStandardNewMacro( vtkWorkflowTask );


vtkWorkflowTask
::vtkWorkflowTask()
{
  this->Name = "";
  this->Instruction = "";
  this->Next = "";
  this->Prerequisite = "";
  this->Recovery = "";
}


vtkWorkflowTask
::~vtkWorkflowTask()
{
  this->Name = "";
  this->Instruction = "";
  this->Next = "";
  this->Prerequisite = "";
  this->Recovery = "";
}


void vtkWorkflowTask
::Copy( vtkWorkflowTask* otherTask )
{
  if ( otherTask == NULL )
  {
    return;
  }

  this->SetName( otherTask->GetName() );
  this->SetInstruction( otherTask->GetInstruction() );
  this->SetNext( otherTask->GetNext() );
  this->SetPrerequisite( otherTask->GetPrerequisite() );
  this->SetRecovery( otherTask->GetRecovery() );
}


std::string vtkWorkflowTask
::ToXMLString( vtkIndent indent )
{
  std::stringstream xmlstring;

  xmlstring << indent << "<Task";
  xmlstring << " Name=\"" << this->Name << "\"";
  xmlstring << " Instruction=\"" << this->Instruction << "\"";
  xmlstring << " Next=\"" << this->Next << "\"";
  xmlstring << " Prerequisite=\"" << this->Prerequisite << "\"";
  xmlstring << " Recovery=\"" << this->Recovery << "\"";
  xmlstring << " />" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowTask
::FromXMLElement( vtkXMLDataElement* element )
{

  if ( strcmp( element->GetName(), "Task" ) != 0 )
  {
    return;  // If it's not a "Task" or is the wrong tool jump to the next.
  }

  if ( element->GetAttribute( "Name" ) != NULL )
  {
    this->Name = std::string( element->GetAttribute( "Name" ) );
  }
  if ( element->GetAttribute( "Instruction" ) != NULL )
  {
    this->Instruction = std::string( element->GetAttribute( "Instruction" ) );
  }
  if ( element->GetAttribute( "Next" ) != NULL )
  {
    this->Next = std::string( element->GetAttribute( "Next" ) );
  }
  if ( element->GetAttribute( "Prerequisite" ) != NULL )
  {
    this->Prerequisite = std::string( element->GetAttribute( "Prerequisite" ) );
  }
  if ( element->GetAttribute( "Recovery" ) != NULL )
  {
    this->Recovery = std::string( element->GetAttribute( "Recovery" ) );
  }

}