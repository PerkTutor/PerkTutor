
#include "vtkMRMLWorkflowToolNode.h"

// Constants ------------------------------------------------------------------
static const char* WORKFLOW_PROCEDURE_REFERENCE_ROLE = "ProcedureDefinition";
static const char* WORKFLOW_INPUT_REFERENCE_ROLE = "ProcedureInput";
static const char* WORKFLOW_TRAINING_REFERENCE_ROLE = "ProcedureTraining";

// Standard MRML Node Methods ------------------------------------------------------------

vtkMRMLWorkflowToolNode* vtkMRMLWorkflowToolNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowToolNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowToolNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowToolNode();
}


vtkMRMLNode* vtkMRMLWorkflowToolNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLWorkflowToolNode" );
  if( ret )
    {
      return ( vtkMRMLWorkflowToolNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLWorkflowToolNode();
}



void vtkMRMLWorkflowToolNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


void vtkMRMLWorkflowToolNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);
  
  vtkIndent indent(nIndent);
  
  of << indent << "Name=\"" << this->Name << "\"";
}


void vtkMRMLWorkflowToolNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);
  
  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
  {
    attName  = *(atts++);
    attValue = *(atts++);
 
    if ( ! strcmp( attName, "Name" ) )
    {
      this->Name = std::string( attValue );
    }
  }
}


void vtkMRMLWorkflowToolNode
::Copy( vtkMRMLNode *anode )
{
  this->vtkMRMLNode::Copy( anode ); // this will copy all of the node references
  vtkMRMLWorkflowToolNode *node = ( vtkMRMLWorkflowToolNode* ) anode;
  if ( node == NULL )
  {
    return;
  }
  
  this->SetName( node->GetName() );
  this->SetDefined( node->GetDefined() );
  this->Setinputted( node->GetInputted() );
  this->SetTrained( node->GetTrained() );
}



// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLWorkflowToolNode
::vtkMRMLWorkflowToolNode()
{
  this->Name = "";
  this->Defined = false;
  this->Inputted = false;
  this->Trained = false;
}


vtkMRMLWorkflowToolNode
::~vtkMRMLWorkflowToolNode()
{
}


bool vtkMRMLWorkflowToolNode
::GetDefined()
{
  return ( this->GetWorkflowProcedureNode() != NULL );
}


bool vtkMRMLWorkflowToolNode
::GetInputted()
{
  return ( this->GetWorkflowInputNode() != NULL );
}


bool vtkMRMLWorkflowToolNode
::GetTrained()
{
  return ( this->GetWorkflowTrainingNode() != NULL );
}

std::string vtkMRMLWorkflowToolNode
::GetNodeReferenceIDString( std::string referenceRole )
{
  const char* refID = this->GetNodeReferenceID( referenceRole.c_str() );
  std::string refIDString;

  if ( refID == NULL )
  {
    refIDString = "";
  }
  else
  {
    refIDString = refID;
  }

  return refIDString;
}


vtkMRMLWorkflowProcedureNode* vtkMRMLWorkflowToolNode
::GetWorkflowProcedureNode()
{
  return GetWorkflowProcedureNode::SafeDownCast( this->GetNodeReference( WORKFLOW_PROCEDURE_REFERENCE_ROLE ) );
}


std::string vtkMRMLWorkflowToolNode
::GetWorkflowProcedureID()
{
  return this->GetNodeReferenceIDString( WORKFLOW_PROCEDURE_REFERENCE_ROLE );
}


void vtkMRMLWorkflowToolNode
::SetWorkflowProcedureID( std::string newWorkflowProcedureID )
{
  this->SetAndObserveNodeReferenceID( WORKFLOW_PROCEDURE_REFERENCE_ROLE, newWorkflowProcedureID.c_str(), events.GetPointer() );
}


vtkMRMLWorkflowInputNode* vtkMRMLWorkflowToolNode
::GetWorkflowInputNode()
{
  return GetWorkflowInputNode::SafeDownCast( this->GetNodeReference( WORKFLOW_INPUT_REFERENCE_ROLE ) );
}


std::string vtkMRMLWorkflowToolNode
::GetWorkflowInputID()
{
  return this->GetNodeReferenceIDString( WORKFLOW_INPUT_REFERENCE_ROLE );
}


void vtkMRMLWorkflowToolNode
::SetWorkflowInputID( std::string newWorkflowInputID )
{
  this->SetAndObserveNodeReferenceID( WORKFLOW_INPUT_REFERENCE_ROLE, newWorkflowInputID.c_str(), events.GetPointer() );
}


vtkMRMLWorkflowProcedureNode* vtkMRMLWorkflowToolNode
::GetWorkflowTrainingNode()
{
  return GetWorkflowTrainingNode::SafeDownCast( this->GetNodeReference( WORKFLOW_TRAINING_REFERENCE_ROLE ) );
}


std::string vtkMRMLWorkflowToolNode
::GetWorkflowTrainingID()
{
  return this->GetNodeReferenceIDString( WORKFLOW_TRAINING_REFERENCE_ROLE );
}


void vtkMRMLWorkflowToolNode
::SetWorkflowTrainingID( std::string newWorkflowTrainingID )
{
  this->SetAndObserveNodeReferenceID( WORKFLOW_TRAINING_REFERENCE_ROLE, newWorkflowTrainingID.c_str(), events.GetPointer() );
}