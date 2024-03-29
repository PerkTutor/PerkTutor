
#include "vtkMRMLWorkflowProcedureNode.h"

// Standard MRML Node Methods ------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLWorkflowProcedureNode);

void vtkMRMLWorkflowProcedureNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


void vtkMRMLWorkflowProcedureNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);
}


void vtkMRMLWorkflowProcedureNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);
}


void vtkMRMLWorkflowProcedureNode
::Copy( vtkMRMLNode *anode )
{
  Superclass::Copy( anode );
  vtkMRMLWorkflowProcedureNode *node = ( vtkMRMLWorkflowProcedureNode* ) anode;
  if ( node == NULL )
  {
    return;
  }

  int startModifyState = this->StartModify();

  this->SetProcedureName( node->GetProcedureName() );
  
  this->Tasks.clear();
  std::map< std::string, vtkSmartPointer< vtkWorkflowTask > >::iterator itr;
  for( itr = node->Tasks.begin(); itr != node->Tasks.end(); itr++ )
  {
    vtkSmartPointer< vtkWorkflowTask > currTask = vtkSmartPointer< vtkWorkflowTask >::New();
    currTask->Copy( itr->second );
    this->AddTask( currTask );
  }

  this->EndModify( startModifyState );
}


// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLWorkflowProcedureNode
::vtkMRMLWorkflowProcedureNode()
{
}


vtkMRMLWorkflowProcedureNode
::~vtkMRMLWorkflowProcedureNode()
{
  this->Tasks.clear();
}


int vtkMRMLWorkflowProcedureNode
::GetNumTasks()
{
  return this->Tasks.size();
}


vtkWorkflowTask* vtkMRMLWorkflowProcedureNode
::GetTask( std::string name )
{
  return this->Tasks[ name ];
}


std::vector<std::string> vtkMRMLWorkflowProcedureNode
::GetAllTaskNames()
{
  std::vector< std::string > taskNameVector;
  std::map< std::string, vtkSmartPointer< vtkWorkflowTask > >::iterator itr;  
  for( itr = this->Tasks.begin(); itr != this->Tasks.end(); itr++ )
  {
    taskNameVector.push_back( itr->first );
  }
  return taskNameVector;
}


void vtkMRMLWorkflowProcedureNode
::AddTask( vtkWorkflowTask* newTask )
{
  if ( newTask == NULL )
  {
    return;
  }
  
  this->Tasks[ newTask->GetName() ] = newTask;
  this->Modified();
}


bool vtkMRMLWorkflowProcedureNode
::IsTask( std::string name )
{
  return this->Tasks.find( name ) != this->Tasks.end();
}


std::string vtkMRMLWorkflowProcedureNode
::ToXMLString( vtkIndent indent )
{
  std::stringstream xmlstring;
  
  xmlstring << indent << "<WorkflowProcedure ProcedureName=\"" << this->ProcedureName << "\" >" << std::endl;
    
  std::map< std::string, vtkSmartPointer< vtkWorkflowTask > >::iterator itr;  
  for( itr = this->Tasks.begin(); itr != this->Tasks.end(); itr++ )
  {
    xmlstring << itr->second->ToXMLString( indent.GetNextIndent() );
  }
  
  xmlstring << indent << "</WorkflowProcedure>" << std::endl;

  return xmlstring.str();
}


void vtkMRMLWorkflowProcedureNode
::FromXMLElement( vtkXMLDataElement* element )
{
  if ( element == NULL || strcmp( element->GetName(), "WorkflowProcedure" ) != 0 )
  {
    return;
  }

  if ( element->GetAttribute( "ProcedureName" ) != NULL )
  {
    this->SetProcedureName( element->GetAttribute( "ProcedureName" ) );
  }
  
  int numElements = element->GetNumberOfNestedElements();

  int startModifyState = this->StartModify();

  for ( int i = 0; i < numElements; i++ )
  {
    vtkXMLDataElement* noteElement = element->GetNestedElement( i );

	  vtkSmartPointer< vtkWorkflowTask > newTask = vtkSmartPointer< vtkWorkflowTask >::New();
    newTask->FromXMLElement( noteElement );

	  this->AddTask( newTask );
  }

  this->EndModify( startModifyState );
}