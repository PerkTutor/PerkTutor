

#include "vtkMRMLMetricInstanceNode.h"

// Constants -----------------------------------------------------------------------------
static const char* ASSOCIATED_METRIC_SCRIPT_REFERENCE_ROLE = "AssociatedMetricScript";


// Standard MRML Node Methods ------------------------------------------------------------

vtkMRMLMetricInstanceNode* vtkMRMLMetricInstanceNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLMetricInstanceNode" );
  if( ret )
    {
      return ( vtkMRMLMetricInstanceNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMetricInstanceNode();
}


vtkMRMLNode* vtkMRMLMetricInstanceNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLMetricInstanceNode" );
  if( ret )
    {
      return ( vtkMRMLMetricInstanceNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMetricInstanceNode();
}



void vtkMRMLMetricInstanceNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


void vtkMRMLMetricInstanceNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);
  // Nothing to do - the superclass method takes care of the storage
}


void vtkMRMLMetricInstanceNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);
  // Nothing to do - the superclass method takes care of the storage
}


void vtkMRMLMetricInstanceNode
::Copy( vtkMRMLNode *anode )
{
  Superclass::Copy( anode );
  
  // Nothing to do - the superclass method takes care of the storage
}



// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLMetricInstanceNode
::vtkMRMLMetricInstanceNode()
{
  // Nothing to do
}


vtkMRMLMetricInstanceNode
::~vtkMRMLMetricInstanceNode()
{
  // Nothing to do
}


// The Associated Metric Script Node -----------------------------------------------------------------------------

vtkMRMLMetricScriptNode* vtkMRMLMetricInstanceNode
::GetAssociatedMetricScriptNode()
{
  return vtkMRMLMetricScriptNode::SafeDownCast( this->GetNodeReference( ASSOCIATED_METRIC_SCRIPT_REFERENCE_ROLE ) );
}


std::string vtkMRMLMetricInstanceNode
::GetAssociatedMetricScriptID()
{
  return this->GetNodeReferenceIDString( ASSOCIATED_METRIC_SCRIPT_REFERENCE_ROLE );
}


void vtkMRMLMetricInstanceNode
::SetAssociatedMetricScriptID( std::string newAssociatedMetricScriptID )
{
  this->SetNodeReferenceID( ASSOCIATED_METRIC_SCRIPT_REFERENCE_ROLE, newAssociatedMetricScriptID.c_str() );
}

// Transform and Anatomy Roles ---------------------------------------------------------------

vtkMRMLNode* vtkMRMLMetricInstanceNode
::GetRoleNode( std::string role, RoleTypeEnum roleType )
{
  std::string fullReferenceRole = this->GetFullReferenceRoleName( role, roleType );
  return this->GetNodeReference( fullReferenceRole.c_str() );
}


std::string vtkMRMLMetricInstanceNode
::GetRoleID( std::string role, RoleTypeEnum roleType )
{
  std::string fullReferenceRole = this->GetFullReferenceRoleName( role, roleType );
  return this->GetNodeReferenceIDString( fullReferenceRole.c_str() );
}


void vtkMRMLMetricInstanceNode
::SetRoleID( std::string nodeID, std::string role, RoleTypeEnum roleType )
{
  std::string fullReferenceRole = this->GetFullReferenceRoleName( role, roleType );
  this->SetNodeReferenceID( fullReferenceRole.c_str(), nodeID.c_str() );
}


// Helper methods -----------------------------------------------------------------------------

std::string vtkMRMLMetricInstanceNode
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


std::string vtkMRMLMetricInstanceNode
::GetFullReferenceRoleName( std::string role, RoleTypeEnum roleType )
{
  std::stringstream fullReferenceRoleStream;
  fullReferenceRoleStream << roleType << "/" << role;
  return fullReferenceRoleStream.str();  
}