
#include "vtkMRMLWorkflowProcedureNode.h"

// Standard MRML Node Methods ------------------------------------------------------------

vtkMRMLWorkflowProcedureNode* vtkMRMLWorkflowProcedureNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowProcedureNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowProcedureNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowProcedureNode();
}


vtkMRMLNode* vtkMRMLWorkflowProcedureNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowProcedureNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowProcedureNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowProcedureNode();
}



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

  this->SetProcedureName( node->GetProcedureName() );
  
  this->Tasks.clear();
  std::map< std::string, vtkSmartPointer< vtkWorkflowTask > >::iterator itr;
  for( itr = node->Tasks.begin(); itr != node->Tasks.end(); itr++ )
  {
    vtkSmartPointer< vtkWorkflowTask > currTask = vtkSmartPointer< vtkWorkflowTask >::New();
    currTask->Copy( itr->second );
    this->AddTask( currTask );
  }

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

  this->SetProcedureName( element->GetAttribute( "ProcedureName" ) );
  
  int numElements = element->GetNumberOfNestedElements();

  for ( int i = 0; i < numElements; i++ )
  {
    vtkXMLDataElement* noteElement = element->GetNestedElement( i );

	  vtkSmartPointer< vtkWorkflowTask > newTask = vtkSmartPointer< vtkWorkflowTask >::New();
    newTask->FromXMLElement( noteElement );

	  this->AddTask( newTask );
  }

}