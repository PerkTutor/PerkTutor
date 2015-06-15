
// WorkflowSegmentation MRML includes
#include "vtkMRMLWorkflowSegmentationNode.h"

// Constants ------------------------------------------------------------------
static const char* TOOL_REFERENCE_ROLE = "Tool";


// Constructors and Destructors
// ----------------------------------------------------------------------------

vtkMRMLWorkflowSegmentationNode* vtkMRMLWorkflowSegmentationNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowSegmentationNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowSegmentationNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowSegmentationNode;
}


vtkMRMLNode* vtkMRMLWorkflowSegmentationNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowSegmentationNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowSegmentationNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowSegmentationNode;
}



// Scene: Save and load
// ----------------------------------------------------------------------------


void vtkMRMLWorkflowSegmentationNode::WriteXML( ostream& of, int nIndent )
{
  this->Superclass::WriteXML(of, nIndent);
}



void vtkMRMLWorkflowSegmentationNode::ReadXMLAttributes( const char** atts )
{
  this->Superclass::ReadXMLAttributes(atts);
}



// Slicer Scene
// ----------------------------------------------------------------------------

void vtkMRMLWorkflowSegmentationNode::Copy( vtkMRMLNode *anode )
{  
  this->Superclass::Copy( anode );
}



void vtkMRMLWorkflowSegmentationNode::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


// Constructor/destructor
// ----------------------------------------------------------------------------
vtkMRMLWorkflowSegmentationNode
::vtkMRMLWorkflowSegmentationNode()
{
}



vtkMRMLWorkflowSegmentationNode
::~vtkMRMLWorkflowSegmentationNode()
{
}


// Deal with references to tool nodes
// ----------------------------------------------------------------------------

void vtkMRMLWorkflowSegmentationNode
::AddToolID( std::string toolID )
{
  this->AddAndObserveNodeReferenceID( TOOL_REFERENCE_ROLE, toolID.c_str() );
}


void vtkMRMLWorkflowSegmentationNode
::RemoveToolID( std::string toolID )
{
  // Check all referenced node IDs
  for ( int i = 0; i < this->GetNumberOfNodeReferences( TOOL_REFERENCE_ROLE ); i++ )
  {
    if ( toolID.compare( this->GetNthNodeReferenceID( TOOL_REFERENCE_ROLE, i ) ) == 0 )
    {
      this->RemoveNthNodeReferenceID( TOOL_REFERENCE_ROLE, i );
	    i--;      
	  }
  }
}


std::vector< std::string > vtkMRMLWorkflowSegmentationNode
::GetToolIDs()
{
  std::vector< std::string > toolIDs;

  // Check all referenced node IDs
  for ( int i = 0; i < this->GetNumberOfNodeReferences( TOOL_REFERENCE_ROLE ); i++ )
  {
    toolIDs.push_back( this->GetNthNodeReferenceID( TOOL_REFERENCE_ROLE, i ) );
  }

  return toolIDs;
}
  

bool vtkMRMLWorkflowSegmentationNode
::IsToolID( std::string toolID )
{
  // Check all referenced node IDs
  for ( int i = 0; i < this->GetNumberOfNodeReferences( TOOL_REFERENCE_ROLE ); i++ )
  {
    if ( toolID.compare( this->GetNthNodeReferenceID( TOOL_REFERENCE_ROLE, i ) ) == 0 )
    {
      return true;
    }
  }

  return false;
}


void vtkMRMLWorkflowSegmentationNode
::SetToolIDs( std::vector< std::string > toolIDs )
{
  // Remove all of the active transform IDs
  while( this->GetNumberOfNodeReferences( TOOL_REFERENCE_ROLE ) > 0 )
  {
    this->RemoveNthNodeReferenceID( TOOL_REFERENCE_ROLE, 0 );
  }

  // Add all of the specified IDs
  for ( int i = 0; i < toolIDs.size(); i++ )
  {
    this->AddToolID( toolIDs.at( i ) );
  }
}