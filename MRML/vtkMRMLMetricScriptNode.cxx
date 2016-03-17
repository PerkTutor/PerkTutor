

#include "vtkMRMLMetricScriptNode.h"

// Constants -----------------------------------------------------------------------------

static const char* ASSOCIATED_METRIC_INSTANCE_REFERENCE_ROLE = "AssociatedMetricInstance";


// Standard MRML Node Methods ------------------------------------------------------------

vtkMRMLMetricScriptNode* vtkMRMLMetricScriptNode
::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLMetricScriptNode" );
  if( ret )
    {
      return ( vtkMRMLMetricScriptNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMetricScriptNode();
}


vtkMRMLNode* vtkMRMLMetricScriptNode
::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance( "vtkMRMLMetricScriptNode" );
  if( ret )
    {
      return ( vtkMRMLMetricScriptNode* )ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLMetricScriptNode();
}



void vtkMRMLMetricScriptNode
::PrintSelf( ostream& os, vtkIndent indent )
{
  vtkMRMLNode::PrintSelf(os,indent);
}


void vtkMRMLMetricScriptNode
::WriteXML( ostream& of, int nIndent )
{
  Superclass::WriteXML(of, nIndent);
  // Nothing to do - the superclass method takes care of the storage
}


void vtkMRMLMetricScriptNode
::ReadXMLAttributes( const char** atts )
{
  Superclass::ReadXMLAttributes(atts);
  // Nothing to do - the superclass method takes care of the storage
}


void vtkMRMLMetricScriptNode
::Copy( vtkMRMLNode *anode )
{
  Superclass::Copy( anode );
  
  // Nothing to do - the superclass method takes care of the storage
}



// Constructors and Destructors --------------------------------------------------------------------

vtkMRMLMetricScriptNode
::vtkMRMLMetricScriptNode()
{
  this->PythonSourceCode = "";
}


vtkMRMLMetricScriptNode
::~vtkMRMLMetricScriptNode()
{
  // Nothing to do
}


// Setup Python Sourc Code -----------------------------------------------------------------------------
std::string vtkMRMLMetricScriptNode
::GetPythonSourceCode()
{
  return this->PythonSourceCode;
}


void vtkMRMLMetricScriptNode
::SetPythonSourceCode( std::string newPythonSourceCode )
{
  this->PythonSourceCode = newPythonSourceCode;
  this->InvokeEvent( PythonSourceCodeChangedEvent );
}


// Comparison -----------------------------------------------------------------------------

bool vtkMRMLMetricScriptNode
::IsEqual( vtkMRMLMetricScriptNode* msNode )
{
  if ( this->GetPythonSourceCode().compare( msNode->GetPythonSourceCode() ) != 0 )
  {
    return false;
  }
  return true;
}



// Associated Instance Metrics ---------------------------------------------------------------

// TODO: This isn't used. Do we really need to store references to the metric instances referencing this script?
bool vtkMRMLMetricScriptNode
::IsAssociatedMetricInstanceID( std::string associatedMetricInstanceID )
{
  // Check all referenced node IDs
  for ( int i = 0; i < this->GetNumberOfNodeReferences( ASSOCIATED_METRIC_INSTANCE_REFERENCE_ROLE ); i++ )
  {
    if ( associatedMetricInstanceID.compare( this->GetNthNodeReferenceID( ASSOCIATED_METRIC_INSTANCE_REFERENCE_ROLE, i ) ) == 0 )
    {
      return true;
    }
  }

  return false;
}

// C++ interface -----------------------------------------------------------------------------

/* TODO: Remove?
std::string vtkMRMLMetricScriptNode
::GetMetricName()
{
  this->PythonManager->executeString( QString( "PythonMetricTempVariable = PythonMetricScriptDict[ '%1' ][ 'Class' ].GetMetricName()" ).arg( this->GetID() ) );
  QVariant metricName = this->PythonManager->getVariable( "PythonMetricTempVariable" );
  this->PythonManager->executeString( "del PythonMetricTempVariable" );
  
  return metricName.toStdString();
}


std::string vtkMRMLMetricScriptNode
::GetMetricUnit()
{
  this->PythonManager->executeString( QString( "PythonMetricTempVariable = PythonMetricScriptDict[ '%1' ][ 'Class' ].GetMetricUnit()" ).arg( this->GetID() ) );
  QVariant metricUnit = this->PythonManager->getVariable( "PythonMetricTempVariable" );
  this->PythonManager->executeString( "del PythonMetricTempVariable" );
  
  return metricUnit.toStdString();
}


std::string vtkMRMLMetricScriptNode
::GetAllTransformRoles()
{
  this->PythonManager->executeString( QString( "PythonMetricTempVariable = PythonMetricScriptDict[ '%1' ][ 'Class' ].GetAcceptedTransformRoles()" ).arg( this->GetID() ) );
  QVariant transformRolesVariant = this->PythonManager->getVariable( "PythonMetricTempVariable" );
  this->PythonManager->executeString( "del PythonMetricTempVariable" );
  
  std::vector< std::string > transformRolesVector( transformRolesVariant.length(), "" );
  for ( int i = 0; i < transformRolesVariant.length(); i++ )
  {
    transformRolesVector.at( i ) = transformRolesVariant.at( i ).toStdString();
  }
  return transformRolesVector;
}


std::string vtkMRMLMetricScriptNode
::GetAllAnatomyRoles()
{
  this->PythonManager->executeString( QString( "PythonMetricTempVariable = PythonMetricScriptDict[ '%1' ][ 'Class' ].GetRequiredAnatomyRoles().keys()" ).arg( this->GetID() ) );
  QVariant anatomyRolesVariant = this->PythonManager->getVariable( "PythonMetricTempVariable" );
  this->PythonManager->executeString( "del PythonMetricTempVariable" );
  
  std::vector< std::string > anatomyRolesVector( anatomyRolesVariant.length(), "" );
  for ( int i = 0; i < anatomyRolesVariant.length(); i++ )
  {
    anatomyRolesVector.at( i ) = anatomyRolesVariant.at( i ).toStdString();
  }
  return anatomyRolesVector;
}


std::string vtkMRMLMetricScriptNode
::GetAllAnatomyClassNames()
{
  this->PythonManager->executeString( QString( "PythonMetricTempVariable = PythonMetricScriptDict[ '%1' ][ 'Class' ].GetRequiredAnatomyRoles().keys()" ).arg( this->GetID() ) );
  QVariant anatomyClassNamesVariant = this->PythonManager->getVariable( "PythonMetricTempVariable" );
  this->PythonManager->executeString( "del PythonMetricTempVariable" );
  
  std::vector< std::string > anatomyClassNamesVector( anatomyClassNamesVariant.length(), "" );
  for ( int i = 0; i < anatomyClassNamesVariant.length(); i++ )
  {
    anatomyClassNamesVector.at( i ) = anatomyClassNamesVariant.at( i ).toStdString();
  }
  return anatomyClassNamesVector;
}

*/