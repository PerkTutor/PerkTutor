
#include "vtkWorkflowProcedure.h"

vtkStandardNewMacro( vtkWorkflowProcedure );


vtkWorkflowProcedure
::vtkWorkflowProcedure()
{
}


vtkWorkflowProcedure
::~vtkWorkflowProcedure()
{
  this->Tasks.clear();
}


vtkWorkflowProcedure* vtkWorkflowProcedure
::DeepCopy()
{
  vtkWorkflowProcedure* newWorkflowProcedure = vtkWorkflowProcedure::New();
  newWorkflowProcedure->Name = this->Name;

  for ( int i = 0; i < this->GetNumTasks(); i++ )
  {
    newWorkflowProcedure->Tasks.push_back( this->GetTaskAt(i)->DeepCopy() );
  }

  return newWorkflowProcedure;
}


int vtkWorkflowProcedure
::GetNumTasks()
{
  return this->Tasks.size();
}


vtkWorkflowTask* vtkWorkflowProcedure
::GetTaskAt( int index )
{
  return this->Tasks.at(index);
}


vtkWorkflowTask* vtkWorkflowProcedure
::GetTaskByName( std::string name )
{
  for ( int i = 0; i < this->GetNumTasks(); i++ )
  {
    if ( strcmp( this->GetTaskAt(i)->Name.c_str(), name.c_str() ) == 0 )
	{
      return this->GetTaskAt(i);
	}
  }

  vtkWorkflowTask* task;
  return task;
}


std::vector<std::string> vtkWorkflowProcedure
::GetTaskNames()
{
  std::vector<std::string> taskNameVector;
  for ( int i = 0; i < this->GetNumTasks(); i++ )
  {
    taskNameVector.push_back( this->GetTaskAt(i)->Name );
  }
  return taskNameVector;
}


bool vtkWorkflowProcedure
::IsTask( std::string name )
{
  for ( int i = 0; i < this->GetNumTasks(); i++ )
  {
    if ( name.compare( this->GetTaskAt(i)->Name ) )
	{
      return true;
	}
  }
  return false;
}


int vtkWorkflowProcedure
::IndexByName( std::string name )
{
  for ( int i = 0; i < this->GetNumTasks(); i++ )
  {
    if ( name.compare( this->GetTaskAt(i)->Name ) )
	{
      return i;
	}
  }
  return -1;
}


std::string vtkWorkflowProcedure
::ToXMLString()
{
  std::stringstream xmlstring;

  xmlstring << "<Procedure ";
  xmlstring << " Name=\"" << this->Name << "\""; 
  xmlstring << ">" << std::endl;
  for ( int i = 0; i < this->GetNumTasks(); i++ )
  {
    xmlstring << this->GetTaskAt(i)->ToXMLString();
  }
  xmlstring << "</Procedure>" << std::endl;

  return xmlstring.str();
}


void vtkWorkflowProcedure
::FromXMLElement( vtkXMLDataElement* element )
{
  if ( strcmp( element->GetName(), "Procedure" ) != 0 )
  {
    return;  // If it's not a "log" or is the wrong tool jump to the next.
  }

  this->Name = std::string( element->GetAttribute( "Name" ) );

  vtkWorkflowTask* blankTask;
  this->Tasks = std::vector<vtkWorkflowTask*>( 0, blankTask );

  int numElements = element->GetNumberOfNestedElements();

  for ( int i = 0; i < numElements; i++ )
  {

    vtkXMLDataElement* noteElement = element->GetNestedElement( i );

	vtkWorkflowTask* newTask;
    newTask->FromXMLElement( noteElement );

	this->Tasks.push_back( newTask );

  }
}