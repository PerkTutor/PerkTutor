
#include "vtkWorkflowProcedure.h"

vtkStandardNewMacro( vtkWorkflowProcedure );


vtkWorkflowProcedure
::vtkWorkflowProcedure()
{
  this->Name = "";
}


vtkWorkflowProcedure
::~vtkWorkflowProcedure()
{
  vtkDeleteVector( this->Tasks );
}


vtkWorkflowProcedure* vtkWorkflowProcedure
::DeepCopy()
{
  vtkWorkflowProcedure* newWorkflowProcedure = vtkWorkflowProcedure::New();

  newWorkflowProcedure->Name = this->Name;
  newWorkflowProcedure->Tasks = vtkDeepCopyVector( this->Tasks );

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

  return NULL;
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


void vtkWorkflowProcedure
::AddTask( vtkWorkflowTask* newTask )
{
  this->Tasks.push_back( newTask );
}


bool vtkWorkflowProcedure
::IsTask( std::string name )
{
  for ( int i = 0; i < this->GetNumTasks(); i++ )
  {
    if ( name.compare( this->GetTaskAt(i)->Name ) == 0 )
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
    if ( name.compare( this->GetTaskAt(i)->Name ) == 0 )
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

  for ( int i = 0; i < this->GetNumTasks(); i++ )
  {
    xmlstring << this->GetTaskAt(i)->ToXMLString();
  }

  return xmlstring.str();
}


void vtkWorkflowProcedure
::FromXMLElement( vtkXMLDataElement* element )
{

  this->Name = std::string( element->GetAttribute( "Name" ) );
  int numElements = element->GetNumberOfNestedElements();

  for ( int i = 0; i < numElements; i++ )
  {
    vtkXMLDataElement* noteElement = element->GetNestedElement( i );

	vtkWorkflowTask* newTask = vtkWorkflowTask::New();
    newTask->FromXMLElement( noteElement );

	this->Tasks.push_back( newTask );
  }

}