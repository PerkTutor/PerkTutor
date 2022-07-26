

#include "vtkMRMLMetricInstanceNode.h"

// Constants -----------------------------------------------------------------------------
static const char* ASSOCIATED_METRIC_SCRIPT_REFERENCE_ROLE = "AssociatedMetricScript";
static const char* ROLE_SEPARATOR = "/";


// Standard MRML Node Methods ------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMetricInstanceNode);

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
  this->SetHideFromEditors( true );
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
  this->UpdateNodeName();
}

// Transform and Anatomy Roles ---------------------------------------------------------------

vtkMRMLNode* vtkMRMLMetricInstanceNode
::GetRoleNode( std::string role, /*vtkMRMLMetricInstanceNode::RoleTypeEnum*/ int roleType )
{
  std::string fullReferenceRole = this->GetFullReferenceRoleName( role, roleType );
  return this->GetNodeReference( fullReferenceRole.c_str() );
}


std::string vtkMRMLMetricInstanceNode
::GetRoleID( std::string role, /*vtkMRMLMetricInstanceNode::RoleTypeEnum*/ int roleType )
{
  std::string fullReferenceRole = this->GetFullReferenceRoleName( role, roleType );
  return this->GetNodeReferenceIDString( fullReferenceRole.c_str() );
}


void vtkMRMLMetricInstanceNode
::SetRoleID( std::string nodeID, std::string role, /*vtkMRMLMetricInstanceNode::RoleTypeEnum*/ int roleType )
{
  std::string fullReferenceRole = this->GetFullReferenceRoleName( role, roleType );
  this->SetNodeReferenceID( fullReferenceRole.c_str(), nodeID.c_str() );
  this->UpdateNodeName();
}


std::string vtkMRMLMetricInstanceNode
::GetCombinedRoleString()
{
  std::stringstream roleStringStream;
  for( NodeReferencesType::iterator itr = this->NodeReferences.begin(); itr != this->NodeReferences.end(); itr++ )
  {
    std::string currentRole ( itr->first ); 
    vtkMRMLNode* nodeReference = this->GetNodeReference( currentRole.c_str() );
    int separatorLocation = currentRole.find( ROLE_SEPARATOR );
    if ( separatorLocation == std::string::npos || itr->first.find( ASSOCIATED_METRIC_SCRIPT_REFERENCE_ROLE ) != std::string::npos || nodeReference == NULL )
    {
      continue;
    }
    currentRole.erase( currentRole.begin(), currentRole.begin() + separatorLocation + 1 ); // Remove the ugly RoleType enum at the beginning
    roleStringStream << currentRole << " = " << nodeReference->GetName() << ", ";
  }

  std::string roleString = roleStringStream.str();
  if ( roleString.length() > 0 )
  {
    roleString.erase( roleString.end() - 2, roleString.end() ); // Remove the last ", "
  }
  return roleString;
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
::GetFullReferenceRoleName( std::string role, /*vtkMRMLMetricInstanceNode::RoleTypeEnum*/ int roleType )
{
  std::stringstream fullReferenceRoleStream;
  fullReferenceRoleStream << roleType << ROLE_SEPARATOR << role;
  return fullReferenceRoleStream.str();  
}


void vtkMRMLMetricInstanceNode
::UpdateNodeName()
{
  // The node name should be of the form:
  // Tissue Damage [Transform/Needle = StylusTipToStylus, Anatomy/Tissue = TissueModel]
  std::stringstream nodeNameStream;
  vtkMRMLNode* metricScriptNode = this->GetAssociatedMetricScriptNode();
  if ( metricScriptNode != NULL )
  {
    nodeNameStream << metricScriptNode->GetName();
  }

  nodeNameStream << " [";
  nodeNameStream << this->GetCombinedRoleString();
  nodeNameStream << "]";

  this->SetName( nodeNameStream.str().c_str() );
}